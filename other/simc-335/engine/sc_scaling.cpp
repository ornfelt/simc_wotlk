// ==========================================================================
// Dedmonwakeen's Raid DPS/TPS Simulator.
// Send questions to natehieter@gmail.com
// ==========================================================================

#include "simulationcraft.h"

namespace { // ANONYMOUS NAMESPACE ==========================================

// is_scaling_stat ===========================================================

static bool is_scaling_stat( sim_t* sim,
                             int    stat )
{
  if ( ! sim -> scaling -> scale_only_str.empty() )
  {
    std::vector<std::string> stat_list;
    int num_stats = util_t::string_split( stat_list, sim -> scaling -> scale_only_str, ",:;/|" );
    bool found = false;
    for( int i=0; i < num_stats && ! found; i++ )
    {
      found = ( util_t::parse_stat_type( stat_list[ i ] ) == stat );
    }
    if( ! found ) return false;
  }

  for ( player_t* p = sim -> player_list; p; p = p -> next )
  {
    if ( p -> quiet ) continue;
    if ( p -> is_pet() ) continue;

    if ( p -> scales_with[ stat ] ) return true;
  }

  return false;
}

// stat_may_cap ==============================================================

static bool stat_may_cap( int stat )
{
  if ( stat == STAT_HIT_RATING ) return true;
  if ( stat == STAT_EXPERTISE_RATING ) return true;
  return false;
}

// parse_normalize_scale_factors ============================================

static bool parse_normalize_scale_factors( sim_t* sim, 
                                           const std::string& name,
                                           const std::string& value )
{
  if ( name != "normalize_scale_factors" ) return false;

  if ( value == "1" )
  {
    sim -> scaling -> normalize_scale_factors = 1;
  }
  else
  {
    if ( ( sim -> normalized_stat = util_t::parse_stat_type( value ) ) == STAT_NONE )
    {
      return false;
    }

    sim -> scaling -> normalize_scale_factors = 1;
  }

  return true;
}

} // ANONYMOUS NAMESPACE ====================================================

// ==========================================================================
// Scaling
// ==========================================================================

// scaling_t::scaling_t =====================================================

scaling_t::scaling_t( sim_t* s ) :
  sim( s ), baseline_sim( 0 ), ref_sim( 0 ), delta_sim( 0 ),
  scale_stat( STAT_NONE ),
  scale_value( 0 ),
  calculate_scale_factors( 0 ),
  center_scale_delta( 0 ),
  positive_scale_delta( 0 ),
  scale_lag( 0 ),
  scale_factor_noise( 0.0 ),
  normalize_scale_factors( 0 ),
  smooth_scale_factors( 0 ),
  debug_scale_factors( 0 ),
  current_scaling_stat( 0 ),
  num_scaling_stats( 0 ),
  remaining_scaling_stats( 0 )
{}

// scaling_t::progress ======================================================

double scaling_t::progress( std::string& phase )
{
  if ( ! calculate_scale_factors ) return 1.0;

  if ( num_scaling_stats <= 0 ) return 0.0;

  if ( current_scaling_stat <= 0 )
  {
    phase = "Baseline";
    if ( ! baseline_sim ) return 0;
    return baseline_sim -> current_iteration / (double) sim -> iterations;
  }

  phase  = "Scaling - ";
  phase += util_t::stat_type_abbrev( current_scaling_stat );

  if ( ref_sim && delta_sim )
  {
    double stat_progress = ( num_scaling_stats - remaining_scaling_stats ) / (double) num_scaling_stats;

    double   ref_progress =   ref_sim -> current_iteration / (double)   ref_sim -> iterations;
    double delta_progress = delta_sim -> current_iteration / (double) delta_sim -> iterations;

    stat_progress += ( ref_progress + delta_progress ) / ( 2.0 * num_scaling_stats );

    return stat_progress;
  }

  return 1.0;
}

// scaling_t::init_deltas ===================================================

void scaling_t::init_deltas()
{
  for ( int i=ATTRIBUTE_NONE+1; i < ATTRIBUTE_MAX; i++ )
  {
    if ( stats.attribute[ i ] == 0 ) stats.attribute[ i ] = smooth_scale_factors ? 75 : 150;
  }

  if ( stats.spell_power == 0 ) stats.spell_power = smooth_scale_factors ? 75 : 150;

  if ( stats.attack_power             == 0 ) stats.attack_power             =  smooth_scale_factors ?  75 :  150;
  if ( stats.armor_penetration_rating == 0 ) stats.armor_penetration_rating =  smooth_scale_factors ?  75 :  150;

  if ( stats.expertise_rating == 0 ) 
  {
    stats.expertise_rating =  smooth_scale_factors ? -50 : -100;
    if ( positive_scale_delta ) stats.expertise_rating *= -1;
  }

  if ( stats.hit_rating == 0 ) 
  {
    stats.hit_rating = smooth_scale_factors ? -50 : -100;
    if ( positive_scale_delta ) stats.hit_rating *= -1;
  }

  if ( stats.crit_rating  == 0 ) stats.crit_rating  = smooth_scale_factors ?  75 :  150;
  if ( stats.haste_rating == 0 ) stats.haste_rating = smooth_scale_factors ?  75 :  150;

  if ( stats.armor == 0 ) stats.armor = smooth_scale_factors ? 3000 : 6000;

  if ( stats.weapon_dps            == 0 ) stats.weapon_dps            = smooth_scale_factors ? 25 : 50;
  if ( stats.weapon_offhand_dps    == 0 ) stats.weapon_offhand_dps    = smooth_scale_factors ? 25 : 50;

  if( sim -> weapon_speed_scale_factors )
  {
    if ( stats.weapon_speed          == 0 ) stats.weapon_speed = 0.2;
    if ( stats.weapon_offhand_speed  == 0 ) stats.weapon_offhand_speed  = 0.2;
  }
  else
  {
    stats.weapon_speed         = 0;
    stats.weapon_offhand_speed = 0;
  }
}

// scaling_t::analyze_stats =================================================

void scaling_t::analyze_stats()
{
  if ( ! calculate_scale_factors ) return;

  int num_players = ( int ) sim -> players_by_name.size();
  if ( num_players == 0 ) return;

  remaining_scaling_stats = 0;
  for ( int i=0; i < STAT_MAX; i++ )
    if ( is_scaling_stat( sim, i ) && ( stats.get_stat( i ) != 0 ) )
      remaining_scaling_stats++;
  num_scaling_stats = remaining_scaling_stats;

  if ( num_scaling_stats == 0 ) return;

  baseline_sim = sim;
  if ( smooth_scale_factors )
  {
    if( sim -> report_progress )
    {
      util_t::fprintf( stdout, "\nGenerating smooth baseline...\n" );
      fflush( stdout );
    }

    baseline_sim = new sim_t( sim );
    baseline_sim -> scaling -> scale_stat = STAT_MAX-1;
    baseline_sim -> execute();
  }

  for ( int i=0; i < STAT_MAX; i++ )
  {
    if ( sim -> canceled ) break;

    if ( ! is_scaling_stat( sim, i ) ) continue;

    double scale_delta = stats.get_stat( i );
    if ( scale_delta == 0.0 ) continue;

    current_scaling_stat = i;

    if( sim -> report_progress )
    {
      util_t::fprintf( stdout, "\nGenerating scale factors for %s...\n", util_t::stat_type_string( i ) );
      fflush( stdout );
    }

    bool center = center_scale_delta && ! stat_may_cap( i );

    ref_sim = center ? new sim_t( sim ) : baseline_sim;

    delta_sim = new sim_t( sim );
    delta_sim -> scaling -> scale_stat = i;
    delta_sim -> scaling -> scale_value = +scale_delta / ( center ? 2 : 1 );
    delta_sim -> execute();

    if ( center )
    {
      ref_sim -> scaling -> scale_stat = i;
      ref_sim -> scaling -> scale_value = center ? -( scale_delta / 2 ) : 0;
      ref_sim -> execute();
    }
    
    for ( int j=0; j < num_players; j++ )
    {
      player_t* p = sim -> players_by_name[ j ];

      if ( p -> scales_with[ i ] <= 0 ) continue;

      player_t*   ref_p =   ref_sim -> find_player( p -> name() );
      player_t* delta_p = delta_sim -> find_player( p -> name() );

      double divisor = scale_delta;

      if ( divisor < 0.0 ) divisor += ref_p -> over_cap[ i ];

      double f = ( delta_p -> dps - ref_p -> dps ) / divisor;

      if ( fabs( divisor ) < 1.0 ) // For things like Weapon Speed, show the gain per 0.1 speed gain rather than every 1.0.
        f /= 10.0;

      if ( f >= scale_factor_noise ) p -> scaling.set_stat( i, f );
    }

    if ( debug_scale_factors )
    {
      report_t::print_text( sim -> output_file,   ref_sim, true );
      report_t::print_text( sim -> output_file, delta_sim, true );
    }

    if ( ref_sim != baseline_sim && ref_sim != sim ) delete ref_sim;
    delete delta_sim;
    delta_sim = ref_sim = 0;

    remaining_scaling_stats--;
  }

  if ( baseline_sim != sim ) delete baseline_sim;
  baseline_sim = 0;
}

// scaling_t::analyze_lag ===================================================

void scaling_t::analyze_lag()
{
  if ( ! calculate_scale_factors || ! scale_lag ) return;

  int num_players = ( int ) sim -> players_by_name.size();
  if ( num_players == 0 ) return;

  if( sim -> report_progress )
  {
    util_t::fprintf( stdout, "\nGenerating scale factors for lag...\n" );
    fflush( stdout );
  }

  ref_sim = new sim_t( sim );
  ref_sim -> scaling -> scale_stat = STAT_MAX;
  ref_sim -> execute();

  delta_sim = new sim_t( sim );
  delta_sim ->   queue_lag += 0.100;
  delta_sim ->     gcd_lag += 0.100;
  delta_sim -> channel_lag += 0.100;
  delta_sim -> scaling -> scale_stat = STAT_MAX;
  delta_sim -> execute();

  for ( int i=0; i < num_players; i++ )
  {
    player_t*       p =       sim -> players_by_name[ i ];
    player_t*   ref_p =   ref_sim -> find_player( p -> name() );
    player_t* delta_p = delta_sim -> find_player( p -> name() );

    // Calculate DPS difference per millisecond of lag
    double f = ( ref_p -> dps - delta_p -> dps ) / ( ( delta_sim -> queue_lag - sim -> queue_lag ) * 1000 );

    if ( f >= scale_factor_noise ) p -> scaling_lag = f;
  }

  if ( ref_sim != sim ) delete ref_sim;
  delete delta_sim;
  delta_sim = ref_sim = 0;
}

// scaling_t::analyze_gear_weights ==========================================

void scaling_t::analyze_gear_weights()
{
  if ( num_scaling_stats <= 0 ) return;

  for ( player_t* p = sim -> player_list; p; p = p -> next )
  {
    if ( p -> quiet ) continue;

    chart_t::gear_weights_lootrank( p -> gear_weights_lootrank_link,   p );
    chart_t::gear_weights_wowhead ( p -> gear_weights_wowhead_link,    p );
    chart_t::gear_weights_pawn    ( p -> gear_weights_pawn_std_string, p, true  );
    chart_t::gear_weights_pawn    ( p -> gear_weights_pawn_alt_string, p, false );
  }
}

// scaling_t::normalize =====================================================

void scaling_t::normalize()
{
  if ( num_scaling_stats <= 0 ) return;

  for ( player_t* p = sim -> player_list; p; p = p -> next )
  {
    if ( p -> quiet ) continue;

    double divisor = p -> scaling.get_stat( p -> normalize_by() );

    if ( divisor == 0 ) continue;

    for ( int i=0; i < STAT_MAX; i++ )
    {
      if ( p -> scales_with[ i ] == 0 ) continue;

      p -> normalized_scaling.set_stat( i, p -> scaling.get_stat( i ) / divisor );
    }

  }
}

// scaling_t::analyze =======================================================

void scaling_t::analyze()
{
  if ( sim -> canceled ) return;
  init_deltas();
  analyze_stats();
  analyze_lag();
  analyze_gear_weights();
  normalize();

  for ( player_t* p = sim -> player_list; p; p = p -> next )
  {
    if ( p -> quiet ) continue;

    chart_t::scale_factors( p -> scale_factors_chart, p );
  }

}

// scaling_t::get_options ===================================================

int scaling_t::get_options( std::vector<option_t>& options )
{
  option_t scaling_options[] =
  {
    // @option_doc loc=global/scale_factors title="Scale Factors"
    { "calculate_scale_factors",        OPT_BOOL,   &( calculate_scale_factors              ) },
    { "smooth_scale_factors",           OPT_BOOL,   &( smooth_scale_factors                 ) },
    { "normalize_scale_factors",        OPT_FUNC,   ( void* ) ::parse_normalize_scale_factors },
    { "debug_scale_factors",            OPT_BOOL,   &( debug_scale_factors                  ) },
    { "center_scale_delta",             OPT_BOOL,   &( center_scale_delta                   ) },
    { "positive_scale_delta",           OPT_BOOL,   &( positive_scale_delta                 ) },
    { "scale_lag",                      OPT_BOOL,   &( scale_lag                            ) },
    { "scale_factor_noise",             OPT_FLT,    &( scale_factor_noise                   ) },
    { "scale_strength",                 OPT_FLT,    &( stats.attribute[ ATTR_STRENGTH  ]    ) },
    { "scale_agility",                  OPT_FLT,    &( stats.attribute[ ATTR_AGILITY   ]    ) },
    { "scale_stamina",                  OPT_FLT,    &( stats.attribute[ ATTR_STAMINA   ]    ) },
    { "scale_intellect",                OPT_FLT,    &( stats.attribute[ ATTR_INTELLECT ]    ) },
    { "scale_spirit",                   OPT_FLT,    &( stats.attribute[ ATTR_SPIRIT    ]    ) },
    { "scale_spell_power",              OPT_FLT,    &( stats.spell_power                    ) },
    { "scale_attack_power",             OPT_FLT,    &( stats.attack_power                   ) },
    { "scale_expertise_rating",         OPT_FLT,    &( stats.expertise_rating               ) },
    { "scale_armor_penetration_rating", OPT_FLT,    &( stats.armor_penetration_rating       ) },
    { "scale_hit_rating",               OPT_FLT,    &( stats.hit_rating                     ) },
    { "scale_crit_rating",              OPT_FLT,    &( stats.crit_rating                    ) },
    { "scale_haste_rating",             OPT_FLT,    &( stats.haste_rating                   ) },
    { "scale_weapon_dps",               OPT_FLT,    &( stats.weapon_dps                     ) },
    { "scale_weapon_speed",             OPT_FLT,    &( stats.weapon_speed                   ) },
    { "scale_offhand_weapon_dps",       OPT_FLT,    &( stats.weapon_offhand_dps             ) },
    { "scale_offhand_weapon_speed",     OPT_FLT,    &( stats.weapon_offhand_speed           ) },
    { "scale_only",                     OPT_STRING, &( scale_only_str                       ) },
    { NULL, OPT_UNKNOWN, NULL }
  };

  option_t::copy( options, scaling_options );

  return ( int ) options.size();
}

// scaling_t::has_scale_factors =============================================

bool scaling_t::has_scale_factors()
{
  if ( ! calculate_scale_factors ) return false;
  
  for ( int i=0; i < STAT_MAX; i++ )
  {
    if ( stats.get_stat( i ) != 0 )
    {
      return true;
    }
  }

  return false;
}
