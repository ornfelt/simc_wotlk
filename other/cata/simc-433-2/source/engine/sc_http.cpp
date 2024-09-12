// ==========================================================================
// Dedmonwakeen's Raid DPS/TPS Simulator.
// Send questions to natehieter@gmail.com
// ==========================================================================

#include "simulationcraft.h"
#include "utf8.h"

#include <fstream>

#ifdef USE_TR1
#include <unordered_map>
#else
#include <map>
#endif

// Cross-Platform Support for HTTP-Download =================================

// ==========================================================================
// PLATFORM INDEPENDENT SECTION
// ==========================================================================

// http_t::proxy ============================================================

http_t::proxy_t http_t::proxy;

cache::cache_control_t cache::cache_control_t::singleton;

namespace { // ANONYMOUS NAMESPACE ==========================================

static const bool HTTP_CACHE_DEBUG = false;

static const char* const url_cache_file = "simc_cache.dat";
static const double url_cache_version = 3.1;

static mutex_t cache_mutex;

static const unsigned int NETBUFSIZE = 1 << 15;

struct url_cache_entry_t
{
  std::string result;
  std::string last_modified_header;
  cache::era_t modified, validated;

  url_cache_entry_t() :
    modified( cache::INVALID_ERA ), validated( cache::INVALID_ERA )
  {}
};

#ifdef USE_TR1
typedef std::tr1::unordered_map<std::string, url_cache_entry_t> url_db_t;
#else
typedef std::map<std::string, url_cache_entry_t> url_db_t;
#endif // USE_TR1
static url_db_t url_db;

// throttle =================================================================

static void throttle( int seconds )
{
  static std::time_t last = 0;
  while ( true )
  {
    std::time_t now = std::time( NULL );

    if ( last + seconds <= now )
    {
      last = now;
      return;
    }

    thread_t::sleep( static_cast<int>( last + seconds - now ) );
  }
}

// cache_clear ==============================================================

static void cache_clear()
{
  // writer lock
  auto_lock_t lock( cache_mutex );
  url_db.clear();
}

static const char* const cookies =
  "Cookie: loginChecked=1\r\n"
  "Cookie: cookieLangId=en_US\r\n"
  // Skip arenapass 2011 advertisement .. can we please have a sensible
  // API soon?
  "Cookie: int-WOW=1\r\n"
  "Cookie: int-WOW-arenapass2011=1\r\n"
  "Cookie: int-WOW-epic-savings-promo=1\r\n"
  "Cookie: int-WOW-anniversary=1\r\n"
  "Cookie: int-EuropeanInvitational2011=1\r\n"
  "Cookie: int-dec=1\r\n";

#if defined( NO_HTTP )

// ==========================================================================
// NO HTTP-DOWNLOAD SUPPORT
// ==========================================================================

// download =================================================================

static bool download( url_cache_entry_t&,
                      const std::string& )
{
  return false;
}

#elif defined( _MSC_VER ) || defined( __MINGW32__ )

// ==========================================================================
// HTTP-DOWNLOAD FOR WINDOWS
// ==========================================================================
#include <windows.h>
#include <wininet.h>

// download =================================================================

static bool download( url_cache_entry_t& entry,
                      const std::string& url )
{
  class InetWrapper
  {
  public:
    HINTERNET handle;

    explicit InetWrapper( HINTERNET handle_ ) : handle( handle_ ) {}
    ~InetWrapper() { if ( handle ) InternetCloseHandle( handle ); }
    operator HINTERNET () const { return handle; }
  };

  static HINTERNET hINet;
  if ( !hINet )
  {
    // hINet = InternetOpen( L"Firefox/3.0", INTERNET_OPEN_TYPE_PROXY, "proxy-server", NULL, 0 );
    hINet = InternetOpen( L"Firefox/3.0", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0 );
    if ( ! hINet )
      return false;
  }

  std::string wHeaders;
  wHeaders += cookies;

  if ( ! entry.last_modified_header.empty() )
  {
    wHeaders += "If-Modified-Since: ";
    wHeaders += entry.last_modified_header;
    wHeaders += "\r\n";
  }

  std::string wURL = url;
  wURL = util_t::urlencode( wURL );

  InetWrapper hFile( InternetOpenUrlA( hINet, wURL.c_str(), wHeaders.data(), static_cast<DWORD>( wHeaders.length() ),
                                       INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE, 0 ) );
  if ( ! hFile )
    return false;

  char buffer[ NETBUFSIZE ];
  DWORD amount = sizeof( buffer );
  if ( ! HttpQueryInfoA( hFile, HTTP_QUERY_STATUS_CODE, buffer, &amount, 0 ) )
    return false;

  if ( ! std::strcmp( buffer, "304" ) )
  {
    entry.validated = cache::era();
    return true;
  }

  std::string result;
  while ( InternetReadFile( hFile, buffer, sizeof( buffer ), &amount ) )
  {
    if ( amount == 0 )
      break;
    result.append( &buffer[ 0 ], &buffer[ amount ] );
  }

  entry.result = result;
  entry.modified = entry.validated = cache::era();

  entry.last_modified_header.clear();
  amount = sizeof( buffer );
  DWORD index = 0;
  if ( HttpQueryInfoA( hFile, HTTP_QUERY_LAST_MODIFIED, buffer, &amount, &index ) )
    entry.last_modified_header.assign( &buffer[ 0 ], &buffer[ amount ] );

  return true;
}

#else

// ==========================================================================
// HTTP-DOWNLOAD FOR WINDOWS (MinGW Only) and FOR POSIX COMPLIANT PLATFORMS
// ==========================================================================

#if defined( __MINGW32__ )

#include <windows.h>
#include <wininet.h>
#include <Winsock2.h>
#include <io.h> // for POSIX ::close

#else

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#endif

struct SocketWrapper
{
  int fd;

  SocketWrapper() : fd( -1 ) {}
  ~SocketWrapper() { if ( fd >= 0 ) ::close( fd ); }

  operator int () const { return fd; }

  int connect( const std::string& host, unsigned short port );

  int send( const char* buf, std::size_t size )
  { return ::send( fd, buf, size, 0 ); }

  int recv( char* buf, std::size_t size )
  { return ::recv( fd, buf, size, 0 ); }

  void close()
  { ::close( fd ); fd = -1; }
};

int SocketWrapper::connect( const std::string& host, unsigned short port )
{
  struct hostent* h;
  sockaddr_in a;

  a.sin_family = AF_INET;

  if ( http_t::proxy.type == "http" || http_t::proxy.type == "https" )
  {
    h = gethostbyname( http_t::proxy.host.c_str() );
    a.sin_port = htons( http_t::proxy.port );
  }
  else
  {
    h = gethostbyname( host.c_str() );
    a.sin_port = htons( port );
  }
  if ( ! h ) return -1;

  if ( ( fd = ::socket( PF_INET, SOCK_STREAM, IPPROTO_TCP ) ) < 0 )
    return -1;

  std::memcpy( &a.sin_addr, h -> h_addr_list[ 0 ], sizeof( a.sin_addr ) );

  return ::connect( fd, reinterpret_cast<const sockaddr*>( &a ), sizeof( a ) );
}

#ifdef SC_USE_OPENSSL
#include <openssl/ssl.h>

struct SSLWrapper
{
  static SSL_CTX* ctx;

  static void init()
  {
    if ( ! ctx )
    {
      SSL_library_init();
      ctx = SSL_CTX_new( SSLv23_client_method() );
      SSL_CTX_set_mode( ctx, SSL_MODE_AUTO_RETRY );
    }
  }

  SSL* s;
  SSLWrapper() : s( SSL_new( ctx ) ) {}

  ~SSLWrapper() { if ( s ) close(); }

  int open( int fd )
  { return ( ! SSL_set_fd( s, fd ) || SSL_connect( s ) <= 0 ) ? -1 : 0; }

  int read( char* buffer, std::size_t size )
  { return SSL_read( s, buffer, size ); }

  int write( const char* buffer, std::size_t size )
  { return SSL_write( s, buffer, size ); }

  void close()
  { SSL_shutdown( s ); SSL_free( s ); s = 0; }
};

SSL_CTX* SSLWrapper::ctx = 0;
#endif

// parse_url ================================================================

struct url_t
{
  std::string protocol;
  std::string host;
  unsigned short port;
  std::string path;

  static bool split( url_t& split, const std::string& url );
};

bool url_t::split( url_t&             split,
                   const std::string& url )
{
  typedef std::string::size_type pos_t;

  pos_t pos = url.find_first_of( ':' );
  if ( pos == url.npos || pos + 2 > url.length() ||
       url[ pos + 1 ] != '/' || url[ pos + 2 ] != '/' )
    return false;
  split.protocol.assign( url, 0, pos );

  pos += 3;
  pos_t end = url.find_first_of( ":/", pos );
  split.host.assign( url, pos, end - pos );
  pos = end;

  if ( split.protocol == "https" )
    split.port = 443;
  else
    split.port = 80;

  split.path = '/';
  if ( pos >= url.length() )
    return true;

  if ( url[ pos ] == ':' )
  {
    ++pos;
    split.port = static_cast<unsigned short>( strtol( &url[ pos ], 0, 10 ) );
    pos = url.find_first_of( '/', pos );
    if ( pos == url.npos )
      return true;
  }

  ++pos;
  split.path.append( url, pos, url.npos );
  return true;
}

// build_request ============================================================

static std::string build_request( const url_t&       url,
                                  const std::string& last_modified )
{
  // reference : http://tools.ietf.org/html/rfc2616#page-36
  std::stringstream request;
  bool use_proxy = ( http_t::proxy.type == "http" || http_t::proxy.type == "https" );

  request << "GET ";

  if ( use_proxy )
  {
    request << url.protocol << "://" << url.host;

    // append port info only if not the standard port
    if ( url.port != 80 )
      request << ':' << url.port;
  }

  request << url.path << " HTTP/1.0\r\n"
             "User-Agent: Firefox/3.0\r\n"
             "Accept: */*\r\n";

  if ( ! use_proxy )
    request << "Host: " << url.host << "\r\n";

  request << cookies;

  if ( ! last_modified.empty() )
    request << "If-Modified-Since: " << last_modified << "\r\n";

  request << "Connection: close\r\n"
             "\r\n";

  return request.str();
}

// download =================================================================

static bool download( url_cache_entry_t& entry,
                      const std::string& url )
{
#if defined( __MINGW32__ )

  static bool initialized = false;
  if ( ! initialized )
  {
    WSADATA wsa_data;
    WSAStartup( MAKEWORD( 2,2 ), &wsa_data );
    initialized = true;
  }

#endif

#ifdef SC_USE_OPENSSL
  SSLWrapper::init();
#endif

  std::string current_url = url;
  unsigned int redirect = 0;
  static const unsigned int redirect_max = 8;
  bool ssl_proxy = ( http_t::proxy.type == "https" );
  const bool use_proxy = ( ssl_proxy || http_t::proxy.type == "http" );

  // get a page and if we find a redirect update current_url and loop
  while ( true )
  {
    url_t split_url;
    if ( ! url_t::split( split_url, current_url ) )
      return false;

    bool use_ssl = ssl_proxy || ( ! use_proxy && ( split_url.protocol == "https" ) );
#ifndef SC_USE_OPENSSL
    if ( use_ssl )
    {
      // FIXME: report unable to use SSL
      return false;
    }
#endif

    SocketWrapper s;
    if ( s.connect( split_url.host, split_url.port ) < 0 )
      return false;

    std::string request = build_request( split_url, entry.last_modified_header );

    std::string result;
    char buffer[ NETBUFSIZE ];

#ifdef SC_USE_OPENSSL
    if ( use_ssl )
    {
      SSLWrapper ssl;

      if ( ssl.open( s ) < 0 )
        return false;

      if ( ssl.write( request.data(), request.size() ) != static_cast<int>( request.size() ) )
        return false;

      while ( true )
      {
        int r = ssl.read( buffer, sizeof( buffer ) );
        if ( r <= 0 )
          break;
        result.append( &buffer[ 0 ], &buffer[ r ] );
      }
    }
    else
#endif
    {
      if ( s.send( request.data(), request.size() ) != static_cast<int>( request.size() ) )
        return false;

      while ( true )
      {
        int r = s.recv( buffer, sizeof( buffer ) );
        if ( r <= 0 )
          break;
        result.append( &buffer[ 0 ], &buffer[ r ] );
      }
    }

    s.close();

    std::string::size_type pos = result.find( "\r\n\r\n" );
    if ( pos == result.npos )
      return false;

    // reference : http://tools.ietf.org/html/rfc2616#section-14.30
    // Checking for redirects via "Location:" in headers, which applies at least
    // to 301 Moved Permanently, 302 Found, 303 See Other, 307 Temporary Redirect
    std::string::size_type pos_location = result.find( "\r\nLocation: " );
    if ( pos_location < pos )
    {
      if ( ++redirect > redirect_max )
        return false;

      pos_location += 12;
      std::string::size_type pos_line_end = result.find( "\r\n", pos_location );
      current_url.assign( result, pos_location, pos_line_end - pos_location );
    }
    else
    {
      entry.validated = cache::era();
      {
        std::string::size_type pos_304 = result.find( "304" );
        std::string::size_type line_end = result.find( "\r\n" );
        if ( pos_304 < line_end )
        {
          // Item is not modified
          return true;
        }
      }

      entry.modified = cache::era();
      entry.last_modified_header.clear();
      std::string::size_type pos_l_m = result.find( "\r\nLast-Modified: " );
      if ( pos_l_m < pos )
      {
        pos_l_m += 17;
        std::string::size_type pos_l_m_end = result.find( "\r\n", pos_l_m );
        if ( pos_l_m_end < pos )
          entry.last_modified_header.assign( result, pos_l_m, pos_l_m_end - pos_l_m );
      }
      entry.result.assign( result, pos + 4, result.npos );
      return true;
    }
  }
}

#endif

} // ANONYMOUS NAMESPACE ====================================================

// http_t::clear_cache ======================================================

bool http_t::clear_cache( sim_t* sim,
                          const std::string& name,
                          const std::string& value )
{
  assert( name == "http_clear_cache" ); ( void )name;
  if ( value != "0" && ! sim -> parent ) cache_clear();
  return true;
}

// http_t::cache_load =======================================================

void http_t::cache_load()
{
  auto_lock_t lock( cache_mutex );

  try
  {
    std::ifstream file( url_cache_file, std::ios::binary );
    if ( !file ) return;
    file.exceptions( std::ios::eofbit | std::ios::failbit | std::ios::badbit );
    file.unsetf( std::ios::skipws );

    double version;
    file.read( reinterpret_cast<char*>( &version ) , sizeof( version ) );
    if ( version != url_cache_version )
      return;

    std::string url, result, last_modified;

    while ( ! file.eof() )
    {
      uint32_t size;

      file.read( reinterpret_cast<char*>( &size ), sizeof( size ) );
      url.resize( size );
      file.read( &url[ 0 ], size );

      file.read( reinterpret_cast<char*>( &size ), sizeof( size ) );
      last_modified.resize( size );
      file.read( &last_modified[ 0 ], size );

      file.read( reinterpret_cast<char*>( &size ), sizeof( size ) );
      result.resize( size );
      file.read( &result[ 0 ], size );

      url_cache_entry_t& c = url_db[ url ];
      c.result = result;
      c.last_modified_header = last_modified;
      c.modified = c.validated = cache::IN_THE_BEGINNING;
    }
  }
  catch ( ... )
  {}
}

// http_t::cache_save =======================================================

void http_t::cache_save()
{
  try
  {
    std::ofstream file( url_cache_file, std::ios::binary );
    if ( ! file ) return;
    file.exceptions( std::ios::eofbit | std::ios::failbit | std::ios::badbit );

    file.write( reinterpret_cast<const char*>( &url_cache_version ), sizeof( url_cache_version ) );

    for ( url_db_t::const_iterator p = url_db.begin(), e = url_db.end(); p != e; ++p )
    {
      if ( p -> second.validated == cache::INVALID_ERA )
        continue;

      size_t size = p -> first.size();
      file.write( reinterpret_cast<const char*>( &size ), sizeof( size ) );
      file.write( p -> first.data(), size );

      size = p -> second.last_modified_header.size();
      file.write( reinterpret_cast<const char*>( &size ), sizeof( size ) );
      file.write( p -> second.last_modified_header.data(), size );

      size = p -> second.result.size();
      file.write( reinterpret_cast<const char*>( &size ), sizeof( size ) );
      file.write( p -> second.result.data(), size );
    }
  }
  catch ( ... )
  {}
}


// http_t::get ==============================================================

bool http_t::get( std::string&       result,
                  const std::string& url,
                  cache::behavior_t  caching,
                  const std::string& confirmation,
                  int                throttle_seconds )
{
  result.clear();

  std::string encoded_url;
  format( encoded_url, url );

  auto_lock_t lock( cache_mutex );

  url_cache_entry_t& entry = url_db[ encoded_url ];

  if ( HTTP_CACHE_DEBUG )
  {
    std::ofstream http_log( "simc_http_log.txt", std::ios::app );
    std::ostream::sentry s( http_log );
    if ( s )
    {
      http_log << cache::era() << ": get(\"" << url << "\") [";

      if ( entry.validated != cache::INVALID_ERA )
      {
        if ( entry.validated >= cache::era() )
          http_log << "hot";
        else if ( caching != cache::CURRENT )
          http_log << "warm";
        else
          http_log << "cold";
        http_log << ": (" << entry.modified << ", " << entry.validated << ')';
      }
      else
        http_log << "miss";
      if ( caching != cache::ONLY &&
           ( entry.validated == cache::INVALID_ERA ||
             ( caching == cache::CURRENT && entry.validated < cache::era() ) ) )
        http_log << " download";
      http_log << "]\n";
    }
  }

  if ( entry.validated < cache::era() && ( caching == cache::CURRENT || entry.validated == cache::INVALID_ERA ) )
  {
    if ( caching == cache::ONLY )
      return false;

    util_t::printf( "@" ); fflush( stdout );
    throttle( throttle_seconds );

    if ( ! download( entry, encoded_url ) )
      return false;

    if ( HTTP_CACHE_DEBUG && entry.modified < entry.validated )
    {
      std::ofstream http_log( "simc_http_log.txt", std::ios::app );
      http_log << cache::era() << ": Unmodified (" << entry.modified << ", " << entry.validated << ")\n";
    }

    if ( confirmation.size() && ( entry.result.find( confirmation ) == std::string::npos ) )
    {
      //util_t::printf( "\nsimulationcraft: HTTP failed on '%s'\n", url.c_str() );
      //util_t::printf( "%s\n", ( result.empty() ? "empty" : result.c_str() ) );
      //fflush( stdout );
      return false;
    }
  }

  result = entry.result;
  return true;
}

// http_t::format ===========================================================

void http_t::format_( std::string& encoded_url,
                      const std::string& url )
{
  encoded_url = url;
  util_t::urlencode( util_t::str_to_utf8( encoded_url ) );
}

#ifdef UNIT_TEST

#include <iostream>

std::string& armory_t::format( std::string& name, int ) { return name; }
uint32_t spell_id_t::get_school_mask( school_type x ) { return 0; }

int main( int argc, char* argv[] )
{
  if ( argc > 1 )
  {
    for ( int i = 1; i < argc; ++i )
    {
      if ( !strcmp( argv[ i ], "--dump" ) )
      {
        url_db.clear();
        http_t::cache_load();

        for ( auto& i : url_db )
        {
          std::cout << "URL: \"" << i.first << "\" (" << i.second.last_modified_header << ")\n"
                    << i.second.result << '\n';
        }
      }
      else
      {
        std::string result;
        if ( http_t::get( result, argv[ i ], cache::CURRENT ) )
          std::cout << result << '\n';
        else
          std::cout << "Unable to download \"" << argv[ i ] << "\".\n";
      }
    }
  }
  else
  {
    std::string result;

    if ( http_t::get( result, "http://us.battle.net/wow/en/character/llane/pagezero/advanced", cache::CURRENT ) )
      std::cout << result << '\n';
    else
      std::cout << "Unable to download armory data.\n";

    if ( http_t::get( result, "http://www.wowhead.com/item=40328&xml", cache::CURRENT ) )
      std::cout << result << '\n';
    else
      std::cout << "Unable to download wowhead data.\n";
  }

  return 0;
}

#endif
