// ==========================================================================
// Dedmonwakeen's Raid DPS/TPS Simulator.
// Send questions to natehieter@gmail.com
// ==========================================================================

#include "simulationcraft.h"

// ==========================================================================
// Gear Stats
// ==========================================================================

// gear_stats_t::add_stat ===================================================

void gear_stats_t::add_stat( int    stat,
                             double value )
{
  switch ( stat )
  {
  case STAT_NONE: break;

  case STAT_STRENGTH:  attribute[ ATTR_STRENGTH  ] += value; break;
  case STAT_AGILITY:   attribute[ ATTR_AGILITY   ] += value; break;
  case STAT_STAMINA:   attribute[ ATTR_STAMINA   ] += value; break;
  case STAT_INTELLECT: attribute[ ATTR_INTELLECT ] += value; break;
  case STAT_SPIRIT:    attribute[ ATTR_SPIRIT    ] += value; break;

  case STAT_HEALTH: resource[ RESOURCE_HEALTH ] += value; break;
  case STAT_MANA:   resource[ RESOURCE_MANA   ] += value; break;
  case STAT_RAGE:   resource[ RESOURCE_RAGE   ] += value; break;
  case STAT_ENERGY: resource[ RESOURCE_ENERGY ] += value; break;
  case STAT_FOCUS:  resource[ RESOURCE_FOCUS  ] += value; break;
  case STAT_RUNIC:  resource[ RESOURCE_RUNIC  ] += value; break;

  case STAT_SPELL_POWER:       spell_power       += value; break;
  case STAT_SPELL_PENETRATION: spell_penetration += value; break;
  case STAT_MP5:               mp5               += value; break;

  case STAT_ATTACK_POWER:             attack_power             += value; break;
  case STAT_EXPERTISE_RATING:         expertise_rating         += value; break;
  case STAT_ARMOR_PENETRATION_RATING: armor_penetration_rating += value; break;

  case STAT_HIT_RATING:   hit_rating   += value; break;
  case STAT_CRIT_RATING:  crit_rating  += value; break;
  case STAT_HASTE_RATING: haste_rating += value; break;

  case STAT_WEAPON_DPS:   weapon_dps   += value; break;
  case STAT_WEAPON_SPEED: weapon_speed += value; break;

  case STAT_WEAPON_OFFHAND_DPS:    weapon_offhand_dps    += value; break;
  case STAT_WEAPON_OFFHAND_SPEED:  weapon_offhand_speed  += value; break;

  case STAT_ARMOR:          armor          += value; break;
  case STAT_BONUS_ARMOR:    bonus_armor    += value; break;
  case STAT_DEFENSE_RATING: defense_rating += value; break;
  case STAT_DODGE_RATING:   dodge_rating   += value; break;
  case STAT_PARRY_RATING:   parry_rating   += value; break;

  case STAT_BLOCK_RATING: block_rating += value; break;
  case STAT_BLOCK_VALUE:  block_value  += value; break;

  case STAT_MAX: for ( int i=0; i < ATTRIBUTE_MAX; i++ ) { attribute[ i ] += value; }
    break;

  default: assert( 0 );
  }
}

// gear_stats_t::set_stat ===================================================

void gear_stats_t::set_stat( int    stat,
                             double value )
{
  switch ( stat )
  {
  case STAT_NONE: break;

  case STAT_STRENGTH:  attribute[ ATTR_STRENGTH  ] = value; break;
  case STAT_AGILITY:   attribute[ ATTR_AGILITY   ] = value; break;
  case STAT_STAMINA:   attribute[ ATTR_STAMINA   ] = value; break;
  case STAT_INTELLECT: attribute[ ATTR_INTELLECT ] = value; break;
  case STAT_SPIRIT:    attribute[ ATTR_SPIRIT    ] = value; break;

  case STAT_HEALTH: resource[ RESOURCE_HEALTH ] = value; break;
  case STAT_MANA:   resource[ RESOURCE_MANA   ] = value; break;
  case STAT_RAGE:   resource[ RESOURCE_RAGE   ] = value; break;
  case STAT_ENERGY: resource[ RESOURCE_ENERGY ] = value; break;
  case STAT_FOCUS:  resource[ RESOURCE_FOCUS  ] = value; break;
  case STAT_RUNIC:  resource[ RESOURCE_RUNIC  ] = value; break;

  case STAT_SPELL_POWER:       spell_power       = value; break;
  case STAT_SPELL_PENETRATION: spell_penetration = value; break;
  case STAT_MP5:               mp5               = value; break;

  case STAT_ATTACK_POWER:             attack_power             = value; break;
  case STAT_EXPERTISE_RATING:         expertise_rating         = value; break;
  case STAT_ARMOR_PENETRATION_RATING: armor_penetration_rating = value; break;

  case STAT_HIT_RATING:   hit_rating   = value; break;
  case STAT_CRIT_RATING:  crit_rating  = value; break;
  case STAT_HASTE_RATING: haste_rating = value; break;

  case STAT_WEAPON_DPS:   weapon_dps   = value; break;
  case STAT_WEAPON_SPEED: weapon_speed = value; break;

  case STAT_WEAPON_OFFHAND_DPS:    weapon_offhand_dps    = value; break;
  case STAT_WEAPON_OFFHAND_SPEED:  weapon_offhand_speed  = value; break;

  case STAT_ARMOR:          armor          = value; break;
  case STAT_BONUS_ARMOR:    bonus_armor    = value; break;
  case STAT_DEFENSE_RATING: defense_rating = value; break;
  case STAT_DODGE_RATING:   dodge_rating   = value; break;
  case STAT_PARRY_RATING:   parry_rating   = value; break;

  case STAT_BLOCK_RATING: block_rating = value; break;
  case STAT_BLOCK_VALUE:  block_value  = value; break;

  case STAT_MAX: for ( int i=0; i < ATTRIBUTE_MAX; i++ ) { attribute[ i ] = value; }
    break;

  default: assert( 0 );
  }
}

// gear_stats_t::get_stat ===================================================

double gear_stats_t::get_stat( int stat ) SC_CONST
{
  switch ( stat )
  {
  case STAT_NONE: return 0;

  case STAT_STRENGTH:  return attribute[ ATTR_STRENGTH  ];
  case STAT_AGILITY:   return attribute[ ATTR_AGILITY   ];
  case STAT_STAMINA:   return attribute[ ATTR_STAMINA   ];
  case STAT_INTELLECT: return attribute[ ATTR_INTELLECT ];
  case STAT_SPIRIT:    return attribute[ ATTR_SPIRIT    ];

  case STAT_HEALTH: return resource[ RESOURCE_HEALTH ];
  case STAT_MANA:   return resource[ RESOURCE_MANA   ];
  case STAT_RAGE:   return resource[ RESOURCE_RAGE   ];
  case STAT_ENERGY: return resource[ RESOURCE_ENERGY ];
  case STAT_FOCUS:  return resource[ RESOURCE_FOCUS  ];
  case STAT_RUNIC:  return resource[ RESOURCE_RUNIC  ];

  case STAT_SPELL_POWER:       return spell_power;
  case STAT_SPELL_PENETRATION: return spell_penetration;
  case STAT_MP5:               return mp5;

  case STAT_ATTACK_POWER:             return attack_power;
  case STAT_EXPERTISE_RATING:         return expertise_rating;
  case STAT_ARMOR_PENETRATION_RATING: return armor_penetration_rating;

  case STAT_HIT_RATING:   return hit_rating;
  case STAT_CRIT_RATING:  return crit_rating;
  case STAT_HASTE_RATING: return haste_rating;

  case STAT_WEAPON_DPS:   return weapon_dps;
  case STAT_WEAPON_SPEED: return weapon_speed;

  case STAT_WEAPON_OFFHAND_DPS:    return weapon_offhand_dps;
  case STAT_WEAPON_OFFHAND_SPEED:  return weapon_offhand_speed;

  case STAT_ARMOR:          return armor;
  case STAT_BONUS_ARMOR:    return bonus_armor;
  case STAT_DEFENSE_RATING: return defense_rating;
  case STAT_DODGE_RATING:   return dodge_rating;
  case STAT_PARRY_RATING:   return parry_rating;

  case STAT_BLOCK_RATING: return block_rating;
  case STAT_BLOCK_VALUE:  return block_value;

  default: assert( 0 );
  }
  return 0;
}

// gear_stats_t::print ======================================================

void gear_stats_t::print( FILE* file )
{
  for ( int i=0; i < STAT_MAX; i++ )
  {
    double value = get_stat( i );

    if ( value != 0 )
    {
      util_t::fprintf( file, " %s=%.*f", util_t::stat_type_abbrev( i ), ( ( ( value - ( int ) value ) > 0 ) ? 3 : 0 ), value );
    }
  }
  util_t::fprintf( file, "\n" );
}

// gear_stats_t::stat_mod ===================================================

double gear_stats_t::stat_mod( int stat )
{
  switch ( stat )
  {
  case STAT_MP5:               return 2.50;
  case STAT_ATTACK_POWER:      return 0.50;
  case STAT_SPELL_POWER:       return 0.86;
  case STAT_SPELL_PENETRATION: return 0.80;
  }
  return 1.0;
}
