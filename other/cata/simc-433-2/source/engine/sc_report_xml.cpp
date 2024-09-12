// ==========================================================================
// Dedmonwakeen's Raid DPS/TPS Simulator.
// Send questions to natehieter@gmail.com
// ==========================================================================

#include "simulationcraft.h"
#include <stack>

namespace { // ANONYMOUS NAMESPACE ==========================================

class xml_writer_t
{
private:
  FILE* file;
  enum state
  {
    NONE, TAG, TEXT
  };
  std::stack<std::string> current_tags;
  std::string tabulation;
  state current_state;
  std::string indentation;

  struct replacement
  {
    char from;
    const char* to;
  };

public:
  xml_writer_t( const std::string & filename )
    : tabulation( "  " ), current_state( NONE )
  {
    file = fopen( filename.c_str(), "w" );
  }

  ~xml_writer_t()
  {
    if ( file )
    {
      fclose( file );
    }
  }

  bool ready() const { return file != NULL; }

  void set_tabulation( const std::string & tabulation_ )
  {
    int n = indentation.size() / tabulation.size();
    tabulation = tabulation_;
    indentation.clear();
    while ( --n >= 0 )
      indentation += tabulation;
  }

  int printf( const char *format, ... ) const PRINTF_ATTRIBUTE( 2,3 )
  {
    va_list fmtargs;
    va_start( fmtargs, format );

    int retcode = vfprintf( file, format, fmtargs );

    va_end( fmtargs );

    return retcode;
  }

  void init_document( const std::string & stylesheet_file )
  {
    assert( current_state == NONE );

    printf( "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n" );
    if ( !stylesheet_file.empty() )
    {
      printf( "<?xml-stylesheet type=\"text/xml\" href=\"%s\"?>", stylesheet_file.c_str() );
    }

    current_state = TEXT;
  }

  void begin_tag( const std::string & tag )
  {
    assert( current_state != NONE );

    if ( current_state != TEXT )
    {
      printf( ">" );
    }

    printf( "\n%s<%s", indentation.c_str(), tag.c_str() );

    current_tags.push( tag );
    indentation += tabulation;

    current_state = TAG;
  }

  void end_tag()
  {
    assert( current_state != NONE );
    assert( ! current_tags.empty() );
    assert( indentation.size() == tabulation.size() * current_tags.size() );

    std::string tag = current_tags.top();
    current_tags.pop();

    indentation.resize( indentation.size() - tabulation.size() );

    if ( current_state == TAG )
    {
      printf( "/>" );
    }
    else if ( current_state == TEXT )
    {
      printf( "\n%s</%s>", indentation.c_str(), tag.c_str() );
    }

    current_state = TEXT;
  }

  void print_attribute( const std::string & name, const std::string & value )
  { print_attribute_unescaped( name, sanitize( value ) ); }

  void print_attribute_unescaped( const std::string & name, const std::string & value )
  {
    assert( current_state != NONE );

    if ( current_state == TAG )
    {
      printf( " %s=\"%s\"", name.c_str(), value.c_str() );
    }
  }

  void print_tag( const std::string & name, const std::string & inner_value )
  {
    assert( current_state != NONE );

    if ( current_state != TEXT )
    {
      printf( ">" );
    }

    printf( "\n%s<%s>%s</%s>", indentation.c_str(), name.c_str(), sanitize( inner_value ).c_str(), name.c_str() );

    current_state = TEXT;
  }

  void print_text( const std::string & input )
  {
    assert( current_state != NONE );

    if ( current_state != TEXT )
    {
      printf( ">" );
    }

    printf( "\n%s", sanitize( input ).c_str() );

    current_state = TEXT;
  }

  static std::string sanitize( std::string v )
  {
    static const replacement replacements[] =
    {
      { '&', "&amp;" },
      { '"', "&quot;" },
      { '<', "&lt;" },
      { '>', "&gt;" },
    };

    for ( unsigned int i = 0; i < sizeof_array( replacements ); ++i )
      util_t::replace_all( v, replacements[ i ].from, replacements[ i ].to );

    return v;
  }
};

// report_t::print_xml ======================================================

void print_xml_errors( sim_t* sim, xml_writer_t & writer );
void print_xml_raid_events( sim_t* sim, xml_writer_t & writer );
void print_xml_roster( sim_t* sim, xml_writer_t & writer );
void print_xml_targets( sim_t* sim, xml_writer_t & writer );
void print_xml_buffs( sim_t* sim, xml_writer_t & writer );
void print_xml_hat_donors( sim_t* sim, xml_writer_t & writer );
void print_xml_performance( sim_t* sim, xml_writer_t & writer );
void print_xml_summary( sim_t* sim, xml_writer_t & writer );
void print_xml_player( sim_t* sim, xml_writer_t & writer, player_t * p, player_t * owner );

void print_xml_player_stats( xml_writer_t & writer, player_t * p );
void print_xml_player_attribute( xml_writer_t & writer, const std::string& attribute, double initial, double gear, double buffed );
void print_xml_player_actions( xml_writer_t & writer, player_t* p );
void print_xml_player_action_definitions( xml_writer_t & writer, player_t * p );
void print_xml_player_buffs( xml_writer_t & writer, player_t * p );
void print_xml_player_uptime( xml_writer_t & writer, player_t * p );
void print_xml_player_procs( xml_writer_t & writer, player_t * p );
void print_xml_player_gains( xml_writer_t & writer, player_t * p );
void print_xml_player_scale_factors( xml_writer_t & writer, player_t * p );
void print_xml_player_dps_plots( xml_writer_t & writer, player_t * p );
void print_xml_player_charts( xml_writer_t & writer, player_t * p );

void print_xml_errors( sim_t* sim, xml_writer_t & writer )
{
  size_t num_errors = sim -> error_list.size();
  if ( num_errors > 0 )
  {
    writer.begin_tag( "errors" );
    for ( size_t i=0; i < num_errors; i++ )
    {
      writer.begin_tag( "error" );
      writer.print_attribute( "message", sim -> error_list[ i ] );
      writer.end_tag(); // </error>
    }
    writer.end_tag(); // </errors>
  }
}

void print_xml_raid_events( sim_t* sim, xml_writer_t & writer )
{
  if ( ! sim -> raid_events_str.empty() )
  {
    writer.begin_tag( "raid_events" );

    std::vector<std::string> raid_event_names;
    int num_raid_events = util_t::string_split( raid_event_names, sim -> raid_events_str, "/" );

    for ( int i=0; i < num_raid_events; i++ )
    {
      writer.begin_tag( "raid_event" );
      writer.print_attribute( "index", util_t::to_string( i ) );
      writer.print_attribute( "name", raid_event_names[i] );
      writer.end_tag(); // </raid_event>
    }

    writer.end_tag(); // </raid_events>
  }
}

void print_xml_roster( sim_t* sim, xml_writer_t & writer )
{
  writer.begin_tag( "players" );

  int num_players = ( int ) sim -> players_by_dps.size();
  for ( int i = 0; i < num_players; ++i )
  {
    player_t * current_player = sim -> players_by_name[ i ];
    print_xml_player( sim, writer, current_player, NULL );
    for ( pet_t* pet = sim -> players_by_name[ i ] -> pet_list; pet; pet = pet -> next_pet )
    {
      if ( pet -> summoned )
        print_xml_player( sim, writer, pet, current_player );
    }
  }

  writer.end_tag(); // </players>
}

void print_xml_targets( sim_t* sim, xml_writer_t & writer )
{
  writer.begin_tag( "targets" );

  size_t count = sim -> targets_by_name.size();
  for ( size_t i = 0; i < count; ++i )
  {
    player_t * current_player = sim -> targets_by_name[ i ];
    print_xml_player( sim, writer, current_player, NULL );
    for ( pet_t* pet = current_player -> pet_list; pet; pet = pet -> next_pet )
    {
      if ( pet -> summoned )
        print_xml_player( sim, writer, pet, current_player );
    }
  }

  writer.end_tag(); // </targets>
}

void print_xml_player( sim_t * sim, xml_writer_t & writer, player_t * p, player_t * owner )
{
  writer.begin_tag( "player" );
  writer.print_attribute( "name", p -> name() );
  if ( owner )
    writer.print_attribute( "owner", owner -> name() );
  writer.print_tag( "type", p -> is_enemy() ? "Target" : p -> is_add() ? "Add" : "Player" );
  writer.print_tag( "level", util_t::to_string( p -> level ) );
  writer.print_tag( "race", p -> race_str.c_str() );
  writer.begin_tag( "class" );
  writer.print_attribute( "type", util_t::player_type_string( p->type ) );
  if ( p -> is_pet() )
    writer.print_attribute( "subtype", util_t::pet_type_string( p -> cast_pet() -> pet_type ) );
  writer.end_tag(); // </class>
  writer.print_tag( "talent_tree", util_t::talent_tree_string( p -> primary_tree() ) );
  writer.print_tag( "primary_role", util_t::role_type_string( p -> primary_role() ) );
  writer.print_tag( "position", p -> position_str );
  writer.begin_tag( "dps" );
  writer.print_attribute( "value", util_t::to_string( p -> dps.mean ) );
  writer.print_attribute( "effective", util_t::to_string( p -> dpse.mean ) );
  writer.print_attribute( "error", util_t::to_string( p -> dps_error ) );
  writer.print_attribute( "range", util_t::to_string( ( p -> dps.max - p -> dps.min ) / 2.0 ) );
  writer.print_attribute( "convergence", util_t::to_string( p -> dps_convergence ) );
  writer.end_tag(); // </dps>

  if ( p -> rps_loss > 0 )
  {
    writer.begin_tag( "dpr" );
    writer.print_attribute( "value", util_t::to_string( p -> dpr ) );
    writer.print_attribute( "rps_loss", util_t::to_string( p -> rps_loss ) );
    writer.print_attribute( "rps_gain", util_t::to_string( p -> rps_gain ) );
    writer.print_attribute( "resource", util_t::resource_type_string( p -> primary_resource() ) );
    writer.end_tag(); // </dpr>
  }

  writer.begin_tag( "waiting_time" );
  writer.print_attribute( "pct", util_t::to_string( p -> fight_length.mean ? 100.0 * p -> waiting_time.mean / p -> fight_length.mean : 0 ) );
  writer.end_tag(); // </waiting_time>
  writer.begin_tag( "active_time" );
  writer.print_attribute( "pct", util_t::to_string( sim -> simulation_length.mean ? p -> fight_length.mean / sim -> simulation_length.mean * 100.0 : 0 ) );
  writer.end_tag(); // </active_time>
  writer.print_tag( "apm", util_t::to_string( p -> fight_length.mean ? 60.0 * p -> executed_foreground_actions.mean / p -> fight_length.mean : 0 ) );

  if ( p -> origin_str.compare( "unknown" ) )
    writer.print_tag( "origin", p -> origin_str.c_str() );

  if ( ! p -> talents_str.empty() )
    writer.print_tag( "talents_url", p -> talents_str.c_str() );

  print_xml_player_stats( writer, p );
  print_xml_player_actions( writer, p );

  print_xml_player_buffs( writer, p );
  print_xml_player_uptime( writer, p );
  print_xml_player_procs( writer, p );
  print_xml_player_gains( writer, p );
  print_xml_player_scale_factors( writer, p );
  print_xml_player_dps_plots( writer, p );
  print_xml_player_charts( writer, p );

  writer.end_tag(); // </player>
}

void print_xml_player_stats( xml_writer_t & writer, player_t * p )
{
  print_xml_player_attribute( writer, "strength",
                              p -> strength(),  p -> stats.attribute[ ATTR_STRENGTH  ], p -> attribute_buffed[ ATTR_STRENGTH  ] );
  print_xml_player_attribute( writer, "agility",
                              p -> agility(),   p -> stats.attribute[ ATTR_AGILITY   ], p -> attribute_buffed[ ATTR_AGILITY   ] );
  print_xml_player_attribute( writer, "stamina",
                              p -> stamina(),   p -> stats.attribute[ ATTR_STAMINA   ], p -> attribute_buffed[ ATTR_STAMINA   ] );
  print_xml_player_attribute( writer, "intellect",
                              p -> intellect(), p -> stats.attribute[ ATTR_INTELLECT ], p -> attribute_buffed[ ATTR_INTELLECT ] );
  print_xml_player_attribute( writer, "spirit",
                              p -> spirit(),    p -> stats.attribute[ ATTR_SPIRIT    ], p -> attribute_buffed[ ATTR_SPIRIT    ] );
  print_xml_player_attribute( writer, "mastery",
                              p -> composite_mastery(), p -> stats.mastery_rating, p -> buffed_mastery );
  print_xml_player_attribute( writer, "spellpower",
                              p -> composite_spell_power( SCHOOL_MAX ) * p -> composite_spell_power_multiplier(), p -> stats.spell_power, p -> buffed_spell_power );
  print_xml_player_attribute( writer, "spellhit",
                              100 * p -> composite_spell_hit(), p -> stats.hit_rating, 100 * p -> buffed_spell_hit );
  print_xml_player_attribute( writer, "spellcrit",
                              100 * p -> composite_spell_crit(), p -> stats.crit_rating, 100 * p -> buffed_spell_crit );
  print_xml_player_attribute( writer, "spellpenetration",
                              100 * p -> composite_spell_penetration(), p -> stats.spell_penetration, 100 * p -> buffed_spell_penetration );
  print_xml_player_attribute( writer, "spellhaste",
                              100 * ( 1 / p -> spell_haste - 1 ), p -> stats.haste_rating, 100 * ( 1 / p -> buffed_spell_haste - 1 ) );
  print_xml_player_attribute( writer, "mp5",
                              p -> composite_mp5(), p -> stats.mp5, p -> buffed_mp5 );
  print_xml_player_attribute( writer, "attackpower",
                              p -> composite_attack_power() * p -> composite_attack_power_multiplier(), p -> stats.attack_power, p -> buffed_attack_power );
  print_xml_player_attribute( writer, "attackhit",
                              100 * p -> composite_attack_hit(), p -> stats.hit_rating, 100 * p -> buffed_attack_hit );
  print_xml_player_attribute( writer, "attackcrit",
                              100 * p -> composite_attack_crit(), p -> stats.crit_rating, 100 * p -> buffed_attack_crit );
  print_xml_player_attribute( writer, "expertise",
                              100 * p -> composite_attack_expertise(), p -> stats.expertise_rating, 100 * p -> buffed_attack_expertise );
  print_xml_player_attribute( writer, "attackhaste",
                              100 * ( 1 / p -> composite_attack_haste() - 1 ), p -> stats.haste_rating, 100 * ( 1 / p -> buffed_attack_haste - 1 ) );
  print_xml_player_attribute( writer, "attackspeed",
                              100 * ( 1 / p -> composite_attack_speed() - 1 ), p -> stats.haste_rating, 100 * ( 1 / p -> buffed_attack_speed - 1 ) );
  print_xml_player_attribute( writer, "armor",
                              p -> composite_armor(), ( p -> stats.armor + p -> stats.bonus_armor ), p -> buffed_armor );
  print_xml_player_attribute( writer, "miss",
                              100 * ( p -> composite_tank_miss( SCHOOL_PHYSICAL ) ), 0, 100 * p -> buffed_miss );
  print_xml_player_attribute( writer, "dodge",
                              100 * ( p -> composite_tank_dodge() - p -> diminished_dodge() ), p -> stats.dodge_rating, 100 * p -> buffed_dodge );
  print_xml_player_attribute( writer, "parry",
                              100 * ( p -> composite_tank_parry() - p -> diminished_parry() ), p -> stats.parry_rating, 100 * p -> buffed_parry );
  print_xml_player_attribute( writer, "block",
                              100 * p -> composite_tank_block(), p -> stats.block_rating, 100 * p -> buffed_block );
  print_xml_player_attribute( writer, "tank_crit",
                              100 * p -> composite_tank_crit( SCHOOL_PHYSICAL ), 0, 100 * p -> buffed_crit );

  writer.begin_tag( "resource" );
  writer.print_attribute( "name", "health" );
  writer.print_attribute( "base", util_t::to_string( p -> resource_max[ RESOURCE_HEALTH ], 0 ) );
  writer.print_attribute( "buffed", util_t::to_string( p -> resource_buffed[ RESOURCE_HEALTH ], 0 ) );
  writer.end_tag(); // </resource>

  writer.begin_tag( "resource" );
  writer.print_attribute( "name", "mana" );
  writer.print_attribute( "base", util_t::to_string( p -> resource_max[ RESOURCE_MANA ], 0 ) );
  writer.print_attribute( "buffed", util_t::to_string( p -> resource_buffed[ RESOURCE_MANA ], 0 ) );
  writer.end_tag(); // </resource>
}

void print_xml_player_attribute( xml_writer_t & writer, const std::string & attribute, double initial, double gear, double buffed )
{
  writer.begin_tag( "attribute" );
  writer.print_attribute( "name", attribute );
  writer.print_attribute( "base", util_t::to_string( initial, 0 ) );
  writer.print_attribute( "gear", util_t::to_string( gear, 0 ) );
  writer.print_attribute( "buffed", util_t::to_string( buffed, 0 ) );
  writer.end_tag(); // </attribute>
}

void print_xml_player_actions( xml_writer_t & writer, player_t* p )
{
  writer.begin_tag( "glyphs" );
  std::vector<std::string> glyph_names;
  int num_glyphs = util_t::string_split( glyph_names, p -> glyphs_str.c_str(), "/" );
  for ( int i=0; i < num_glyphs; i++ )
  {
    writer.begin_tag( "glyph" );
    writer.print_attribute( "name", glyph_names[i] );
    writer.end_tag(); // </glyph>
  }
  writer.end_tag(); // </glyphs>

  writer.begin_tag( "priorities" );
  std::vector<std::string> action_list;
  int num_actions = util_t::string_split( action_list, p -> action_list_str, "/" );
  for ( int i=0; i < num_actions; i++ )
  {
    writer.begin_tag( "action" );
    writer.print_attribute( "index", util_t::to_string( i ) );
    writer.print_attribute( "value", action_list[ i ] );
    writer.end_tag(); // </action>
  }
  writer.end_tag(); // </priorities>


  writer.begin_tag( "actions" );

  for ( stats_t* s = p -> stats_list; s; s = s -> next )
  {
    if ( s -> num_executes > 1 || s -> compound_amount > 0 )
    {
      int id = 0;
      for ( action_t* a = s -> player -> action_list; a; a = a -> next )
      {
        if ( a -> stats != s ) continue;
        id = a -> id;
        if ( ! a -> background ) break;
      }

      writer.begin_tag( "action" );
      writer.print_attribute( "id", util_t::to_string( id ) );
      writer.print_attribute( "name", s -> name_str );
      writer.print_attribute( "count", util_t::to_string( s -> num_executes ) );
      writer.print_attribute( "frequency", util_t::to_string( s -> frequency ) );
      writer.print_attribute( "dpe", util_t::to_string( s -> ape, 0 ) );
      writer.print_attribute( "dpe_pct", util_t::to_string( s -> portion_amount * 100.0 ) );
      writer.print_attribute( "dpet", util_t::to_string( s -> apet ) );
      writer.print_attribute( "apr", util_t::to_string( s -> apr ) );
      writer.print_attribute( "pdps", util_t::to_string( s -> portion_aps.mean ) );

      if ( ! s -> timeline_aps_chart.empty() )
      {
        writer.begin_tag( "chart" );
        writer.print_attribute( "type", "timeline_aps" );
        writer.print_attribute( "href", s -> timeline_aps_chart );
        writer.end_tag(); // </chart>
      }

      writer.print_tag( "etpe", util_t::to_string( s -> etpe ) );
      writer.print_tag( "ttpt", util_t::to_string( s -> ttpt ) );
      writer.print_tag( "actual_amount", util_t::to_string( s -> actual_amount.mean ) );
      writer.print_tag( "total_amount", util_t::to_string( s -> total_amount.mean ) );
      writer.print_tag( "overkill_pct", util_t::to_string( s -> overkill_pct ) );
      writer.print_tag( "aps", util_t::to_string( s -> aps ) );
      writer.print_tag( "apet", util_t::to_string( s -> apet ) );

      if ( s -> num_direct_results > 0 )
      {
        writer.begin_tag( "direct_results" );
        writer.print_attribute( "count", util_t::to_string( s -> num_direct_results ) );

        for ( int i=RESULT_MAX-1; i >= RESULT_NONE; i-- )
        {
          if ( s -> direct_results[ i ].count.mean )
          {
            writer.begin_tag( "result" );
            writer.print_attribute( "type", util_t::result_type_string( i ) );
            writer.print_attribute( "count", util_t::to_string( s -> direct_results[ i ].count.mean ) );
            writer.print_attribute( "pct", util_t::to_string( s -> direct_results[ i ].pct ) );
            writer.print_attribute( "min", util_t::to_string( s -> direct_results[ i ].actual_amount.min ) );
            writer.print_attribute( "max", util_t::to_string( s -> direct_results[ i ].actual_amount.max ) );
            writer.print_attribute( "avg", util_t::to_string( s -> direct_results[ i ].actual_amount.mean ) );
            writer.print_attribute( "avg_min", util_t::to_string( s -> direct_results[ i ].avg_actual_amount.min ) );
            writer.print_attribute( "avg_max", util_t::to_string( s -> direct_results[ i ].avg_actual_amount.max ) );
            writer.print_attribute( "actual", util_t::to_string( s -> direct_results[ i ].fight_actual_amount.mean ) );
            writer.print_attribute( "total", util_t::to_string( s -> direct_results[ i ].fight_total_amount.mean ) );
            writer.print_attribute( "overkill_pct", util_t::to_string( s -> direct_results[ i ].overkill_pct ) );
            writer.end_tag(); // </result>
          }
        }

        writer.end_tag(); // </direct_result>
      }

      if ( s -> num_ticks > 0 )
      {
        writer.begin_tag( "tick_results" );
        writer.print_attribute( "count", util_t::to_string( s -> num_tick_results ) );
        writer.print_attribute( "ticks", util_t::to_string( s -> num_ticks ) );

        for ( int i=RESULT_MAX-1; i >= RESULT_NONE; i-- )
        {
          if ( s -> tick_results[ i ].count.mean )
          {
            writer.begin_tag( "result" );
            writer.print_attribute( "type", util_t::result_type_string( i ) );
            writer.print_attribute( "count", util_t::to_string( s -> tick_results[ i ].count.mean ) );
            writer.print_attribute( "pct", util_t::to_string( s -> tick_results[ i ].pct ) );
            writer.print_attribute( "min", util_t::to_string( s -> tick_results[ i ].actual_amount.min ) );
            writer.print_attribute( "max", util_t::to_string( s -> tick_results[ i ].actual_amount.max ) );
            writer.print_attribute( "avg", util_t::to_string( s -> tick_results[ i ].actual_amount.mean ) );
            writer.print_attribute( "avg_min", util_t::to_string( s -> tick_results[ i ].avg_actual_amount.min ) );
            writer.print_attribute( "avg_max", util_t::to_string( s -> tick_results[ i ].avg_actual_amount.max ) );
            writer.print_attribute( "actual", util_t::to_string( s -> tick_results[ i ].fight_actual_amount.mean ) );
            writer.print_attribute( "total", util_t::to_string( s -> tick_results[ i ].fight_total_amount.mean ) );
            writer.print_attribute( "overkill_pct", util_t::to_string( s -> tick_results[ i ].overkill_pct ) );
            writer.end_tag(); // </result>
          }
        }

        writer.end_tag(); // </tick_result>
      }


      writer.begin_tag( "chart" );
      writer.print_attribute( "type", "timeline_stat_aps" );
      writer.print_attribute_unescaped( "href", s -> timeline_aps_chart );
      writer.end_tag(); // </chart>

      // Action Details
      std::vector<std::string> processed_actions;
      size_t size = s -> action_list.size();
      for ( size_t i = 0; i < size; i++ )
      {
        action_t* a = s -> action_list[ i ];

        bool found = false;
        size_t size_processed = processed_actions.size();
        for ( size_t j = 0; j < size_processed && !found; j++ )
          if ( processed_actions[ j ] == a -> name() )
            found = true;
        if ( found ) continue;
        processed_actions.push_back( a -> name() );

        if ( a -> target )
        {
          writer.print_tag( "target", a -> target -> name() );
        }
      }
      writer.end_tag(); // </action>
    }
  }

  writer.end_tag(); // </actions>

  print_xml_player_action_definitions( writer, p );
}

void print_xml_player_buffs( xml_writer_t & writer, player_t * p )
{
  writer.begin_tag( "buffs" );

  for ( buff_t* b = p -> buff_list; b; b = b -> next )
  {
    if ( b -> quiet || ! b -> start_count )
      continue;

    writer.begin_tag( "buff" );
    writer.print_attribute( "name", b -> name() );
    writer.print_attribute( "type", b -> constant ? "constant" : "dynamic" );

    if ( b -> constant )
    {
      writer.print_attribute( "start", util_t::to_string( b -> avg_start, 1 ) );
      writer.print_attribute( "refresh", util_t::to_string( b -> avg_refresh, 1 ) );
      writer.print_attribute( "interval", util_t::to_string( b -> avg_start_interval, 1 ) );
      writer.print_attribute( "trigger", util_t::to_string( b -> avg_trigger_interval, 1 ) );
      writer.print_attribute( "uptime", util_t::to_string( b -> uptime_pct.mean, 0 ) );

      if ( b -> benefit_pct > 0 && b -> benefit_pct < 100 )
      {
        writer.print_attribute( "benefit", util_t::to_string( b -> benefit_pct ) );
      }
    }
    writer.end_tag(); // </buff>
  }

  writer.end_tag(); // </buffs>
}

void print_xml_player_uptime( xml_writer_t & writer, player_t * p )
{
  writer.begin_tag( "benefits" );

  for ( benefit_t* u = p -> benefit_list; u; u = u -> next )
  {
    if ( u -> ratio > 0 )
    {
      writer.begin_tag( "benefit" );
      writer.print_attribute( "name", u -> name() );
      writer.print_attribute( "ratio_pct", util_t::to_string( u -> ratio * 100.0, 1 ) );
      writer.end_tag();
    }
  }

  writer.end_tag(); // </benefits>


  writer.begin_tag( "uptimes" );

  for ( uptime_t* u = p -> uptime_list; u; u = u -> next )
  {
    if ( u -> uptime > 0 )
    {
      writer.begin_tag( "uptime" );
      writer.print_attribute( "name", u -> name_str );
      writer.print_attribute( "pct", util_t::to_string( u -> uptime * 100.0, 1 ) );
      writer.end_tag();
    }
  }

  writer.end_tag(); // </uptimes>
}

void print_xml_player_procs( xml_writer_t & writer, player_t * p )
{
  writer.begin_tag( "procs" );

  for ( proc_t* proc = p -> proc_list; proc; proc = proc -> next )
  {
    if ( proc -> count > 0 )
    {
      writer.begin_tag( "proc" );
      writer.print_attribute( "name", proc -> name() );
      writer.print_attribute( "count", util_t::to_string( proc -> count, 1 ) );
      writer.print_attribute( "frequency", util_t::to_string( proc -> frequency, 2 ) );
      writer.end_tag(); // </proc>
    }
  }

  writer.end_tag(); // </procs>
}

void print_xml_player_gains( xml_writer_t & writer, player_t * p )
{
  writer.begin_tag( "gains" );

  for ( gain_t* g = p -> gain_list; g; g = g -> next )
  {
    if ( g -> actual > 0 || g -> overflow > 0 )
    {
      writer.begin_tag( "gain" );
      writer.print_attribute( "name", g -> name() );
      writer.print_attribute( "actual", util_t::to_string( g -> actual, 1 ) );
      double overflow_pct = 100.0 * g -> overflow / ( g -> actual + g -> overflow );
      if ( overflow_pct > 1.0 )
        writer.print_attribute( "overflow_pct", util_t::to_string( overflow_pct, 1 ) );
      writer.end_tag();
    }
  }

  writer.end_tag();
}

void print_xml_player_scale_factors( xml_writer_t & writer, player_t * p )
{
  if ( ! p -> sim -> scaling -> has_scale_factors() ) return;

  if ( p -> is_add() || p -> is_enemy() ) return;

  if ( p -> sim -> report_precision < 0 )
    p -> sim -> report_precision = 2;

  writer.begin_tag( "scale_factors" );

  gear_stats_t& sf = p -> scaling;
  gear_stats_t& sf_norm = p -> scaling_normalized;

  writer.begin_tag( "weights" );

  for ( int i=0; i < STAT_MAX; i++ )
  {
    if ( p -> scales_with[ i ] != 0 )
    {
      writer.begin_tag( "stat" );
      writer.print_attribute( "name", util_t::stat_type_abbrev( i ) );
      writer.print_attribute( "value", util_t::to_string( sf.get_stat( i ), p -> sim -> report_precision ) );
      writer.print_attribute( "normalized", util_t::to_string( sf_norm.get_stat( i ), p -> sim -> report_precision ) );
      writer.print_attribute( "scaling_error", util_t::to_string( p -> scaling_error.get_stat( i ), p -> sim -> report_precision ) );
      writer.print_attribute( "delta", util_t::to_string( p -> sim -> scaling -> stats.get_stat( i ) ) );

      writer.end_tag(); // </stat>
    }
  }

  size_t num_scaling_stats = p -> scaling_stats.size();
  for ( size_t i=0; i < num_scaling_stats; i++ )
  {
    writer.begin_tag( "scaling_stat" );
    writer.print_attribute( "name", util_t::stat_type_abbrev( p -> scaling_stats[ i ] ) );
    writer.print_attribute( "index", util_t::to_string( ( int64_t )i ) );

    if ( i > 0 )
    {
      if ( ( ( p -> scaling.get_stat( p -> scaling_stats[ i - 1 ] ) - p -> scaling.get_stat( p -> scaling_stats[ i ] ) )
             > sqrt ( p -> scaling_compare_error.get_stat( p -> scaling_stats[ i - 1 ] ) * p -> scaling_compare_error.get_stat( p -> scaling_stats[ i - 1 ] ) / 4 + p -> scaling_compare_error.get_stat( p -> scaling_stats[ i ] ) * p -> scaling_compare_error.get_stat( p -> scaling_stats[ i ] ) / 4 ) * 2 ) )
        writer.print_attribute( "relative_to_previous", ">" );
      else
        writer.print_attribute( "relative_to_previous", "=" );
    }

    writer.end_tag(); // </scaling_stat>
  }

  writer.end_tag(); // </weights>

  if ( p -> sim -> scaling -> normalize_scale_factors )
  {
    writer.begin_tag( "dps_per_point" );
    writer.print_attribute( "stat", util_t::stat_type_abbrev( p -> normalize_by() ) );
    writer.print_attribute( "value", util_t::to_string( p -> scaling.get_stat( p -> normalize_by() ), p -> sim -> report_precision ) );
    writer.end_tag(); // </dps_per_point>
  }
  if ( p -> sim -> scaling -> scale_lag )
  {
    writer.begin_tag( "scale_lag_ms" );
    writer.print_attribute( "value", util_t::to_string( p -> scaling_lag, p -> sim -> report_precision ) );
    writer.print_attribute( "error", util_t::to_string( p -> scaling_lag_error, p -> sim -> report_precision ) );
    writer.end_tag(); // </scale_lag_ms>
  }


  std::string lootrank   = p -> gear_weights_lootrank_link;
  std::string wowhead    = p -> gear_weights_wowhead_link;
  std::string wowreforge = p -> gear_weights_wowreforge_link;
  //std::string pawn_std   = p -> gear_weights_pawn_std_string;
  //std::string pawn_alt   = p -> gear_weights_pawn_alt_string;

  writer.begin_tag( "link" );
  writer.print_attribute( "name", "wowhead" );
  writer.print_attribute( "type", "ranking" );
  writer.print_attribute_unescaped( "href", wowhead );
  writer.end_tag(); // </link>

  writer.begin_tag( "link" );
  writer.print_attribute( "name", "lootrank" );
  writer.print_attribute( "type", "ranking" );
  writer.print_attribute( "href", lootrank );
  writer.end_tag(); // </link>

  writer.begin_tag( "link" );
  writer.print_attribute( "name", "wowreforge" );
  writer.print_attribute( "type", "optimizer" );
  writer.print_attribute( "href", wowreforge );
  writer.end_tag(); // </link>

  writer.end_tag(); // </scale_factors>
}

void print_xml_player_dps_plots( xml_writer_t & writer, player_t * p )
{
  sim_t* sim = p -> sim;

  if ( sim -> plot -> dps_plot_stat_str.empty() ) return;

  int range = sim -> plot -> dps_plot_points / 2;

  double min = -range * sim -> plot -> dps_plot_step;
  double max = +range * sim -> plot -> dps_plot_step;

  int points = 1 + range * 2;

  writer.begin_tag( "dps_plot_data" );
  writer.print_attribute( "min", util_t::to_string( min, 1 ) );
  writer.print_attribute( "max", util_t::to_string( max, 1 ) );
  writer.print_attribute( "points", util_t::to_string( points ) );

  for ( int i=0; i < STAT_MAX; i++ )
  {
    std::vector<double>& pd = p -> dps_plot_data[ i ];

    if ( ! pd.empty() )
    {
      writer.begin_tag( "dps" );
      writer.print_attribute( "stat", util_t::stat_type_abbrev( i ) );
      size_t num_points = pd.size();
      for ( size_t j=0; j < num_points; j++ )
      {
        writer.print_tag( "value", util_t::to_string( pd[ j ], 0 ) );
      }
      writer.end_tag(); // </dps>
    }
  }

  writer.end_tag(); // </dps_plot_data>
}

void print_xml_player_charts( xml_writer_t & writer, player_t * p )
{
  writer.begin_tag( "charts" );

  if ( ! p -> action_dpet_chart.empty() )
  {
    writer.begin_tag( "chart" );
    writer.print_attribute( "type", "dpet" );
    writer.print_attribute_unescaped( "href", p -> action_dmg_chart );
    writer.end_tag(); // </chart>
  }

  if ( ! p -> action_dmg_chart.empty() )
  {
    writer.begin_tag( "chart" );
    writer.print_attribute( "type", "dmg" );
    writer.print_attribute_unescaped( "href", p -> action_dmg_chart );
    writer.end_tag(); // </chart>
  }

  if ( ! p -> scaling_dps_chart.empty() )
  {
    writer.begin_tag( "chart" );
    writer.print_attribute( "type", "scaling_dps" );
    writer.print_attribute_unescaped( "href", p -> scaling_dps_chart );
    writer.end_tag(); // </chart>
  }

  if ( ! p -> reforge_dps_chart.empty() )
  {
    writer.begin_tag( "chart" );
    writer.print_attribute( "type", "reforge_dps" );
    writer.print_attribute_unescaped( "href", p -> reforge_dps_chart );
    writer.end_tag(); // </chart>
  }

  if ( ! p -> scale_factors_chart.empty() )
  {
    writer.begin_tag( "chart" );
    writer.print_attribute( "type", "scale_factors" );
    writer.print_attribute_unescaped( "href", p -> scale_factors_chart );
    writer.end_tag(); // </chart>
  }

  if ( ! p -> timeline_dps_chart.empty() )
  {
    writer.begin_tag( "chart" );
    writer.print_attribute( "type", "timeline_dps" );
    writer.print_attribute_unescaped( "href", p -> timeline_dps_chart );
    writer.end_tag(); // </chart>
  }

  if ( ! p -> distribution_dps_chart.empty() )
  {
    writer.begin_tag( "chart" );
    writer.print_attribute( "type", "distribution_dps" );
    writer.print_attribute_unescaped( "href", p -> distribution_dps_chart );
    writer.end_tag(); // </chart>
  }

  if ( ! p -> time_spent_chart.empty() )
  {
    writer.begin_tag( "chart" );
    writer.print_attribute( "type", "time_spent" );
    writer.print_attribute_unescaped( "href", p -> time_spent_chart );
    writer.end_tag(); // </chart>
  }

  writer.end_tag(); // </charts>
}

void print_xml_buffs( sim_t* sim, xml_writer_t & writer )
{
  writer.begin_tag( "buffs" );

  for ( buff_t* b = sim -> buff_list; b; b = b -> next )
  {
    if ( b -> quiet || ! b -> start_count )
      continue;

    writer.begin_tag( "buff" );
    writer.print_attribute( "name", b -> name() );
    writer.print_attribute( "type", b -> constant ? "constant" : "dynamic" );

    if ( b -> constant )
    {
      writer.print_attribute( "start", util_t::to_string( b -> avg_start, 1 ) );
      writer.print_attribute( "refresh", util_t::to_string( b -> avg_refresh, 1 ) );
      writer.print_attribute( "interval", util_t::to_string( b -> avg_start_interval, 1 ) );
      writer.print_attribute( "trigger", util_t::to_string( b -> avg_trigger_interval, 1 ) );
      writer.print_attribute( "uptime", util_t::to_string( b -> uptime_pct.mean, 0 ) );

      if ( b -> benefit_pct > 0 && b -> benefit_pct < 100 )
      {
        writer.print_attribute( "benefit", util_t::to_string( b -> benefit_pct ) );
      }

      if ( b -> trigger_pct > 0 && b -> trigger_pct < 100 )
      {
        writer.print_attribute( "trigger_pct", util_t::to_string( b -> trigger_pct ) );
      }
    }
    writer.end_tag(); // </buff>
  }

  writer.end_tag(); // </buffs>
}

struct compare_hat_donor_interval
{
  bool operator()( const player_t* l, const player_t* r ) const
  {
    return( l -> procs.hat_donor -> frequency < r -> procs.hat_donor -> frequency );
  }
};

void print_xml_hat_donors( sim_t* sim, xml_writer_t & writer )
{
  std::vector<player_t*> hat_donors;

  int num_players = ( int ) sim -> players_by_name.size();
  for ( int i=0; i < num_players; i++ )
  {
    player_t* p = sim -> players_by_name[ i ];
    if ( p -> procs.hat_donor -> count )
      hat_donors.push_back( p );
  }

  int num_donors = ( int ) hat_donors.size();
  if ( num_donors )
  {
    range::sort( hat_donors, compare_hat_donor_interval()  );

    writer.begin_tag( "honor_among_thieves" );

    for ( int i=0; i < num_donors; i++ )
    {
      writer.begin_tag( "donors" );
      player_t* p = hat_donors[ i ];
      proc_t* proc = p -> procs.hat_donor;
      writer.print_attribute( "name", p -> name() );
      writer.print_attribute( "frequency_sec", util_t::to_string( proc -> frequency, 2 ) );
      writer.print_attribute( "frequency_pct", util_t::to_string( ( 1.0 / proc -> frequency ), 3 ) );
      writer.end_tag(); // </donors>
    }

    writer.end_tag(); // </honor_among_thieves>
  }
}

void print_xml_performance( sim_t* sim, xml_writer_t & writer )
{
  writer.begin_tag( "performance" );

  writer.print_tag( "total_events", util_t::to_string( sim -> total_events_processed ) );
  writer.print_tag( "max_event_queue", util_t::to_string( sim -> max_events_remaining ) );
  writer.print_tag( "target_health", util_t::to_string( sim -> target -> resource_base[ RESOURCE_HEALTH ], 0 ) );
  writer.print_tag( "sim_seconds", util_t::to_string( sim -> iterations * sim -> simulation_length.mean, 0 ) );
  writer.print_tag( "cpu_seconds", util_t::to_string( sim -> elapsed_cpu.total_seconds(), 3 ) );
  writer.print_tag( "speed_up", util_t::to_string( sim -> iterations * sim -> simulation_length.mean / sim -> elapsed_cpu.total_seconds(), 0 ) );
  writer.begin_tag( "rng" );
  writer.print_attribute( "roll", util_t::to_string( ( sim -> rng -> expected_roll  == 0 ) ? 1.0 : ( sim -> rng -> actual_roll  / sim -> rng -> expected_roll  ), 6 ) );
  writer.print_attribute( "range", util_t::to_string( ( sim -> rng -> expected_range == 0 ) ? 1.0 : ( sim -> rng -> actual_range / sim -> rng -> expected_range ), 6 ) );
  writer.print_attribute( "gauss", util_t::to_string( ( sim -> rng -> expected_gauss == 0 ) ? 1.0 : ( sim -> rng -> actual_gauss / sim -> rng -> expected_gauss ), 6 ) );
  writer.end_tag(); // </rng>

  writer.end_tag(); // </performance>
}

void print_xml_config( sim_t* sim, xml_writer_t & writer )
{
  writer.begin_tag( "config" );

  writer.print_tag( "style", sim -> hosted_html ? "hosted" : ( sim -> print_styles ? "print" : "not-specified" ) );
  writer.print_tag( "has_scale_factors", sim -> scaling -> has_scale_factors() ? "true" : "false" );
  writer.print_tag( "report_pets_separately", sim -> report_pets_separately ? "true" : "false" );
  writer.print_tag( "report_details", sim -> report_details ? "true" : "false" );
  writer.print_tag( "report_targets", sim -> report_targets ? "true" : "false" );
  writer.print_tag( "normalize_scale_factors", sim -> scaling ->normalize_scale_factors ? "true" : "false" );

  writer.end_tag(); // </config>
}

void print_xml_summary( sim_t* sim, xml_writer_t & writer )
{
  writer.begin_tag( "summary" );

  time_t rawtime;
  time ( &rawtime );

  writer.print_tag( "timestamp", ctime( &rawtime ) );
  writer.print_tag( "iterations", util_t::to_string( sim -> iterations ) );

  writer.print_tag( "threads", util_t::to_string( sim -> threads < 1 ? 1 : sim -> threads ) );

  writer.print_tag( "confidence", util_t::to_string( sim -> confidence * 100.0 ) );

  writer.begin_tag( "simulation_length" );
  writer.print_attribute( "mean", util_t::to_string( sim -> simulation_length.mean, 0 ) );
  if ( !sim -> fixed_time )
  {
    writer.print_attribute( "min", util_t::to_string( sim -> simulation_length.min, 0 ) );
    writer.print_attribute( "max", util_t::to_string( sim -> simulation_length.max, 0 ) );
  }
  writer.print_attribute( "total", util_t::to_string( sim -> simulation_length.sum, 0 ) );
  writer.end_tag(); // </simulation_length>

  writer.begin_tag( "events" );
  writer.print_attribute( "processed", util_t::to_string( sim -> total_events_processed ) );
  writer.print_attribute( "max_remaining", util_t::to_string( sim -> max_events_remaining ) );
  writer.end_tag(); // </events>

  writer.print_tag( "fight_style", sim -> fight_style );

  writer.print_tag( "elapsed_cpu_sec", util_t::to_string( sim -> elapsed_cpu.total_seconds() ) );

  writer.begin_tag( "lag" );
  writer.print_attribute( "type", "world" );
  writer.print_attribute( "value", util_t::to_string( sim -> world_lag.total_millis() ) );
  writer.print_attribute( "stddev", util_t::to_string( sim -> world_lag_stddev.total_millis() ) );
  writer.end_tag(); // </lag>

  writer.begin_tag( "lag" );
  writer.print_attribute( "type", "queue" );
  writer.print_attribute( "value", util_t::to_string( sim -> queue_lag.total_millis() ) );
  writer.print_attribute( "stddev", util_t::to_string( sim -> queue_lag_stddev.total_millis() ) );
  writer.end_tag(); // </lag>

  if ( sim -> strict_gcd_queue )
  {
    writer.begin_tag( "lag" );
    writer.print_attribute( "type", "gcd" );
    writer.print_attribute( "value", util_t::to_string( sim -> gcd_lag.total_millis() ) );
    writer.print_attribute( "stddev", util_t::to_string( sim -> gcd_lag_stddev.total_millis() ) );
    writer.end_tag(); // </lag>

    writer.begin_tag( "lag" );
    writer.print_attribute( "type", "channel" );
    writer.print_attribute( "value", util_t::to_string( sim -> channel_lag.total_millis() ) );
    writer.print_attribute( "stddev", util_t::to_string( sim -> channel_lag_stddev.total_millis() ) );
    writer.end_tag(); // </lag>

    writer.begin_tag( "lag" );
    writer.print_attribute( "type", "queue_gcd" );
    writer.print_attribute( "value", util_t::to_string( sim -> queue_gcd_reduction.total_millis() ) );
    writer.end_tag(); // </lag>
  }

  size_t count = sim -> dps_charts.size();
  writer.begin_tag( "charts" );
  writer.print_attribute( "max_players_per_chart", util_t::to_string( MAX_PLAYERS_PER_CHART ) );
  for ( size_t i=0; i < count; i++ )
  {
    writer.begin_tag( "chart" );
    writer.print_attribute( "type", "dps" );
    writer.print_attribute_unescaped( "img_src", sim -> dps_charts[ i ] );
    writer.end_tag(); // </chart>
  }
  count = sim -> hps_charts.size();
  for ( size_t i=0; i < count; i++ )
  {
    writer.begin_tag( "chart" );
    writer.print_attribute( "type", "hps" );
    writer.print_attribute_unescaped( "img_src", sim -> hps_charts[ i ] );
    writer.end_tag(); // </chart>
  }
  count = sim -> gear_charts.size();
  for ( size_t i=0; i < count; i++ )
  {
    writer.begin_tag( "chart" );
    writer.print_attribute( "type", "gear" );
    writer.print_attribute_unescaped( "img_src", sim -> gear_charts[ i ] );
    writer.end_tag(); // </chart>
  }
  count = sim -> dpet_charts.size();
  for ( size_t i=0; i < count; i++ )
  {
    writer.begin_tag( "chart" );
    writer.print_attribute( "type", "dpet" );
    writer.print_attribute_unescaped( "img_src", sim -> dpet_charts[ i ] );
    writer.end_tag(); // </chart>
  }
  writer.end_tag();

  writer.begin_tag( "dmg" );
  writer.print_attribute( "total", util_t::to_string( sim -> total_dmg.mean, 0 ) );
  writer.print_attribute( "dps", util_t::to_string( sim -> raid_dps.mean, 0 ) );
  writer.end_tag(); // </dmg>

  writer.begin_tag( "heal" );
  writer.print_attribute( "total", util_t::to_string( sim -> total_heal.mean, 0 ) );
  writer.print_attribute( "hps", util_t::to_string( sim -> raid_hps.mean, 0 ) );
  writer.end_tag(); // </heal>

  writer.begin_tag( "player_by_dps" );
  count = sim -> players_by_dps.size();
  for ( size_t i = 0; i < count; i++ )
  {
    player_t* p = sim -> players_by_dps[ i ];
    writer.begin_tag( "player" );
    writer.print_attribute( "name", p -> name() );
    writer.print_attribute( "index", util_t::to_string( i ) );
    writer.print_attribute( "dps", util_t::to_string( p -> dps.mean ) );
    writer.end_tag(); // </player>
  }
  writer.end_tag(); // </player_by_dps>

  writer.begin_tag( "player_by_hps" );
  count = sim -> players_by_dps.size();
  for ( size_t i = 0; i < count; i++ )
  {
    player_t* p = sim -> players_by_dps[ i ];
    writer.begin_tag( "player" );
    writer.print_attribute( "name", p -> name() );
    writer.print_attribute( "index", util_t::to_string( i ) );
    writer.print_attribute( "hps", util_t::to_string( p -> hps.mean ) );
    writer.end_tag(); // </player>
  }
  writer.end_tag(); // </player_by_hps>

  print_xml_raid_events( sim, writer );

  writer.end_tag(); // </summary>
}

void print_xml_get_action_list( sim_t* sim, player_t* p, std::map<int, action_t*> & all_actions )
{
  for ( stats_t* s = p -> stats_list; s; s = s -> next )
  {
    if ( s -> num_executes > 1 || s -> compound_amount > 0 || sim -> debug )
    {
      // Action Details
      std::vector<std::string> processed_actions;
      size_t size = s -> action_list.size();
      for ( size_t i = 0; i < size; i++ )
      {
        action_t* a = s -> action_list[ i ];

        bool found = false;
        size_t size_processed = processed_actions.size();
        for ( size_t j = 0; j < size_processed && !found; j++ )
          if ( processed_actions[ j ] == a -> name() )
            found = true;
        if ( found ) continue;
        processed_actions.push_back( a -> name() );
        if ( all_actions.find( a -> id ) == all_actions.end() ) // Not found.
        {
          all_actions[ a -> id ] = a;
        }
      }
    }
  }
}

void print_xml_player_action_definitions( xml_writer_t & writer, player_t * p )
{
  writer.begin_tag( "action_definitions" );

  for ( stats_t* s = p -> stats_list; s; s = s -> next )
  {
    if ( s -> num_executes > 1 || s -> compound_amount > 0 )
    {
      // Action Details
      std::vector<std::string> processed_actions;
      size_t size = s -> action_list.size();
      for ( size_t i = 0; i < size; i++ )
      {
        action_t* a = s -> action_list[ i ];

        bool found = false;
        size_t size_processed = processed_actions.size();
        for ( size_t j = 0; j < size_processed && !found; j++ )
          if ( processed_actions[ j ] == a -> name() )
            found = true;
        if ( found ) continue;
        processed_actions.push_back( a -> name() );

        writer.begin_tag( "action_detail" );
        writer.print_attribute( "id", util_t::to_string( a -> id ) );
        writer.print_attribute( "name", a -> name() );
        writer.print_tag( "school", util_t::school_type_string( a-> school ) );
        writer.print_tag( "resource", util_t::resource_type_string( a -> resource ) );
        writer.print_tag( "tree", util_t::talent_tree_string( a -> tree ) );
        writer.print_tag( "range", util_t::to_string( a -> range ) );
        writer.print_tag( "travel_speed", util_t::to_string( a -> travel_speed ) );
        writer.print_tag( "trigger_gcd", util_t::to_string( a -> trigger_gcd.total_seconds() ) );
        writer.print_tag( "base_cost", util_t::to_string( a -> base_cost ) );
        writer.begin_tag( "cooldown" );
        writer.print_attribute( "duration", util_t::to_string( a -> cooldown -> duration.total_seconds() ) );
        writer.end_tag(); // </cooldown>
        writer.print_tag( "base_execute_time", util_t::to_string( a -> base_execute_time.total_seconds() ) );
        writer.print_tag( "base_crit", util_t::to_string( a -> base_crit ) );
        if ( a -> target )
        {
          writer.print_tag( "target", a -> target -> name() );
        }
        if ( a -> tooltip() )
        {
          writer.print_tag( "tooltip", a -> tooltip() );
        }
        writer.print_tag( "description", report_t::encode_html( a -> desc() ) );

        if ( a -> direct_power_mod || a -> base_dd_min || a -> base_dd_max )
        {
          writer.begin_tag( "direct_damage" );
          writer.print_tag( "may_crit", a -> may_crit ? "true" : "false" );
          writer.print_tag( "direct_power_mod", util_t::to_string( a -> direct_power_mod ) );
          writer.begin_tag( "base" );
          writer.print_attribute( "min", util_t::to_string( a -> base_dd_min ) );
          writer.print_attribute( "max", util_t::to_string( a -> base_dd_max ) );
          writer.end_tag(); // </base>
          writer.end_tag(); // </direct_damage>
        }

        if ( a -> num_ticks )
        {
          writer.begin_tag( "damage_over_time" );
          writer.print_tag( "tick_may_crit", a -> tick_may_crit ? "true" : "false" );
          writer.print_tag( "tick_zero", a -> tick_zero ? "true" : "false" );
          writer.print_tag( "tick_power_mod", util_t::to_string( a -> tick_power_mod ) );
          writer.print_tag( "base", util_t::to_string( a -> base_td ) );
          writer.print_tag( "num_ticks", util_t::to_string( a -> num_ticks ) );
          writer.print_tag( "base_tick_time", util_t::to_string( a -> base_tick_time.total_seconds() ) );
          writer.print_tag( "hasted_ticks", util_t::to_string( a -> hasted_ticks ) );
          writer.print_tag( "dot_behavior", a -> dot_behavior==DOT_REFRESH?"DOT_REFRESH":a -> dot_behavior==DOT_CLIP?"DOT_CLIP":"DOT_WAIT" );
          writer.end_tag(); // </damage_over_time>
        }
        // Extra Reporting for DKs
        if ( a -> player -> type == DEATH_KNIGHT )
        {
          writer.begin_tag( "runes" );
          writer.print_tag( "blood", util_t::to_string( a -> rune_cost() & 0x1 ) );
          writer.print_tag( "frost", util_t::to_string( ( a -> rune_cost() >> 4 ) & 0x1 ) );
          writer.print_tag( "unholy", util_t::to_string( ( a -> rune_cost() >> 2 ) & 0x1 ) );
          writer.print_tag( "runic_power_gain", util_t::to_string( a -> rp_gain ) );
          writer.end_tag(); // </runes>
        }
        if ( a -> weapon )
        {
          writer.begin_tag( "weapon" );
          writer.print_tag( "normalize_speed", a -> normalize_weapon_speed ? "true" : "false" );
          writer.print_tag( "power_mod", util_t::to_string( a -> weapon_power_mod ) );
          writer.print_tag( "multiplier", util_t::to_string( a -> weapon_multiplier ) );
          writer.end_tag(); // </weapon>
        }

        writer.end_tag(); // </action_detail>
      }
    }
  }

  writer.end_tag(); // </action_definitions>
}

} // ANONYMOUS NAMESPACE ====================================================


// report_t::print_xml ======================================================

void report_t::print_xml( sim_t* sim )
{
  int num_players = ( int ) sim -> players_by_name.size();

  if ( num_players == 0 ) return;
  if ( sim -> simulation_length.mean == 0 ) return;
  if ( sim -> xml_file_str.empty() ) return;

  xml_writer_t writer( sim -> xml_file_str.c_str() );
  if ( !writer.ready() )
  {
    sim -> errorf( "Unable to open xml file '%s'\n", sim -> xml_file_str.c_str() );
    return;
  }

  writer.init_document( sim -> xml_stylesheet_file_str );
  writer.begin_tag( "simulationcraft" );

  writer.print_attribute( "major_version", SC_MAJOR_VERSION );
  writer.print_attribute( "minor_version", SC_MINOR_VERSION );
  writer.print_attribute( "wow_version", dbc_t::wow_version( sim -> dbc.ptr ) );
  writer.print_attribute( "ptr", sim -> dbc.ptr ? "true" : "false" );
  writer.print_attribute( "wow_build", dbc_t::build_level( sim -> dbc.ptr ) );

#if SC_BETA

  writer.print_attribute( "beta", "true" );

#endif

  print_xml_config( sim, writer );
  print_xml_summary( sim, writer );

  print_xml_raid_events( sim, writer );
  print_xml_roster( sim, writer );
  print_xml_targets( sim, writer );

  print_xml_buffs( sim, writer );
  print_xml_hat_donors( sim, writer );
  print_xml_performance( sim, writer );

  print_xml_errors( sim, writer );

  writer.end_tag(); // </simulationcraft>
}

