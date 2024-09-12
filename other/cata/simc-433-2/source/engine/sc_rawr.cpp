// ==========================================================================
// Dedmonwakeen's Raid DPS/TPS Simulator.
// Send questions to natehieter@gmail.com
// ==========================================================================

#include "simulationcraft.h"

namespace {  // ANONYMOUS NAMESPACE ==========================================

// translate_glyph_name =====================================================

static const char* translate_glyph_name( player_t* p,
                                         int       index )
{
  switch ( p -> type )
  {
  case MAGE:
    switch ( index )
    {
    case   0: return "arcane_barrage";
    case   1: return "arcane_blast";
    case   2: return "arcane_missiles";
    case   3: return "cone_of_cold";
    case   4: return "deep_freeze";
    case   5: return "fireball";
    case   6: return "frostbolt";
    case   7: return "frostfire";
    case   8: return "ice_lance";
    case   9: return "living_bomb";
    case  10: return "mage_armor";
    case  11: return "molten_armor";
    case  12: return "pyroblast";
    case  13: return "arcane_power";
    case  14: return "blast_wave";
    case  15: return "blink";
    case  16: return "dragons_breath";
    case  17: return "evocation";
    case  18: return "frost_armor";
    case  19: return "frost_nova";
    case  20: return "ice_barrier";
    case  21: return "ice_block";
    case  22: return "icy_veins";
    case  23: return "invisibility";
    case  24: return "mana_shield";
    case  25: return "polymorph";
    case  26: return "slow";
    case  27: return "armors";
    case  28: return "conjuring";
    case  29: return "mirror_image";
    case  30: return "slow_fall";
    case  31: return "monkey";
    case  32: return "penguin";
    default: return 0;
    }
  case DRUID:
    switch ( index )
    {
    case  0: return "berserk";
    case  1: return "insect_swarm";
    case  2: return "lacerate";
    case  3: return "lifebloom";
    case  4: return "mangle";
    case  5: return "moonfire";
    case  6: return "regrowth";
    case  7: return "rejuvenation";
    case  8: return "rip";
    case  9: return "savage_roar";
    case 10: return "shred";
    case 11: return "starfire";
    case 12: return "starsurge";
    case 13: return "swiftmend";
    case 14: return "tigers_fury";
    case 15: return "wrath";
    case 16: return "barkskin";
    case 17: return "entangling_roots";
    case 18: return "faerie_fire";
    case 19: return "feral_charge";
    case 20: return "ferocious_bite";
    case 21: return "focus";
    case 22: return "frenzied_regeneration";
    case 23: return "healing_touch";
    case 24: return "hurricane";
    case 25: return "innervate";
    case 26: return "maul";
    case 27: return "monsoon";
    case 28: return "pounce";
    case 29: return "rebirth";
    case 30: return "solar_beam";
    case 31: return "starfall";
    case 32: return "thorns";
    case 33: return "wild_growth";
    case 34: return "aquatic_form";
    case 35: return "challenging_roar";
    case 36: return "dash";
    case 37: return "mark_of_the_wild";
    case 38: return "treant";
    case 39: return "typhoon";
    case 40: return "unburdened_rebirth";
    default: return 0;
    }
  case PALADIN:
    switch ( index )
    {
    case  0: return "crusader_strike";
    case  1: return "divine_favor";
    case  2: return "exorcism";
    case  3: return "hammer_of_the_righteous";
    case  4: return "holy_shock";
    case  5: return "judgement";
    case  6: return "seal_of_insight";
    case  7: return "seal_of_truth";
    case  8: return "shield_of_righteousness";
    case  9: return "templars_verdict";
    case 10: return "word_of_glory";
    case 11: return "beacon_of_light";
    case 12: return "cleansing";
    case 13: return "consecration";
    case 14: return "dazing_shield";
    case 15: return "divine_plea";
    case 16: return "divine_protection";
    case 17: return "divinity";
    case 18: return "focused_shield";
    case 19: return "hammer_of_justice";
    case 20: return "hammer_of_wrath";
    case 21: return "holy_wrath";
    case 22: return "lay_on_hands";
    //case 23: return "light_of_dawn"; // Rawr has these numbers repeated
    //case 24: return "rebuke";
    case 23: return "salvation";
    case 24: return "the_ascetic_crusader";
    case 25: return "long_word";
    case 26: return "turn_evil";
    case 27: return "blessing_of_kings";
    case 28: return "blessing_of_might";
    case 29: return "insight";
    case 30: return "justice";
    case 31: return "righteousness";
    case 32: return "truth";
    default: return 0;
    }
  case WARRIOR:
    switch ( index )
    {
    case  0: return "bladestorm";
    case  1: return "bloodthirst";
    case  2: return "devastate";
    case  3: return "mortal_strike";
    case  4: return "overpower";
    case  5: return "raging_blow";
    case  6: return "revenge";
    case  7: return "shield_slam";
    case  8: return "slam";
    case  9: return "cleaving";
    case 10: return "colossus_smash";
    case 11: return "death_wish";
    case 12: return "heroic_throw";
    case 13: return "intercept";
    case 14: return "intervene";
    case 15: return "long_charge";
    case 16: return "piercing_howl";
    case 17: return "rapid_charge";
    case 18: return "resonating_power";
    case 19: return "shield_wall";
    case 20: return "shockwave";
    case 21: return "spell_reflection";
    case 22: return "sunder_armor";
    case 23: return "sweeping_strikes";
    case 24: return "thunder_clap";
    case 25: return "victory_rush";
    case 26: return "battle";
    case 27: return "berserker_rage";
    case 28: return "bloody_healing";
    case 29: return "command";
    case 30: return "demoralizing_shout";
    case 31: return "enduring_victory";
    case 32: return "furious_sundering";
    case 33: return "intimidating_shout";
    default: return 0;
    }
  case SHAMAN:
    switch ( index )
    {
    case  0: return "earth_shield";
    case  1: return "earthliving_weapon";
    case  2: return "feral_spirit";
    case  3: return "fire_elemental_totem";
    case  4: return "flame_shock";
    case  5: return "flametongue_weapon";
    case  6: return "lava_burst";
    case  7: return "lava_lash";
    case  8: return "lightning_bolt";
    case  9: return "riptide";
    case 10: return "shocking";
    case 11: return "stormstrike";
    case 12: return "water_shield";
    case 13: return "windfury_weapon";
    case 14: return "chain_heal";
    case 15: return "chain_lightning";
    case 16: return "elemental_mastery";
    case 17: return "fire_nova";
    case 18: return "frost_shock";
    case 19: return "ghost_wolf";
    case 20: return "grounding_totem";
    case 21: return "healing_stream_totem";
    case 22: return "healing_wave";
    case 23: return "hex";
    case 24: return "lightning_shield";
    case 25: return "shamanistic_rage";
    case 26: return "stoneclaw_totem";
    case 27: return "thunder";
    case 28: return "totemic_recall";
    case 29: return "astral_recall";
    case 30: return "renewed_life";
    case 31: return "artic_wolf";
    case 32: return "thunderstorm";
    case 33: return "water_breathing";
    case 34: return "water_walking";
    default: return 0;
    }
  case PRIEST:
    switch ( index )
    {
    case  0: return "dispersion";
    case  1: return "flash_heal";
    case  2: return "guardian_spirit";
    case  3: return "lightwell";
    case  4: return "mind_flay";
    case  5: return "penance";
    case  6: return "power_word_barrier";
    // case  7:
    case  8: return "power_word_shield";
    case  9: return "prayer_of_healing";
    case 10: return "renew";
    // case 11:
    case 12: return "shadow_word_death";
    case 13: return "shadow_word_pain";
    case 14: return "circle_of_healing";
    case 15: return "desperation";
    case 16: return "dispel_magic";
    case 17: return "divine_accuracy";
    case 18: return "fade";
    case 19: return "fear_ward";
    case 20: return "holy_nova";
    case 21: return "inner_fire";
    case 22: return "mass_dispel";
    case 23: return "prayer_of_mending";
    case 24: return "psychic_horror";
    case 25: return "psychic_scream";
    case 26: return "scourge_imprisonment";
    case 27: return "smite";
    case 28: return "spirit_tap";
    case 29: return "fading";
    case 30: return "fortitude";
    case 31: return "levitate";
    case 32: return "shackle_undead";
    case 33: return "shadow_protection";
    case 34: return "shadowfiend";
    default: return 0;
    }
  case DEATH_KNIGHT:
    switch ( index )
    {
    case  0: return "death_and_decay";
    case  1: return "death_coil";
    case  2: return "death_strike";
    case  3: return "frost_strike";
    case  4: return "heart_strike";
    case  5: return "howling_blast";
    case  6: return "icy_touch";
    case  7: return "obliterate";
    case  8: return "raise_dead";
    case  9: return "rune_strike";
    case 10: return "scourge_strike";
    case 11: return "antimagic_shell";
    case 12: return "blood_boil";
    case 13: return "bone_shield";
    case 14: return "chains_of_ice";
    case 15: return "dancing_rune_weapon";
    case 16: return "dark_succor";
    case 17: return "death_grip";
    case 18: return "hungering_cold";
    case 19: return "pestilence";
    case 20: return "pillar_of_frost";
    case 21: return "rune_tap";
    case 22: return "strangulate";
    case 23: return "vampiric_blood";
    case 24: return "blood_tap";
    case 25: return "death_gate";
    case 26: return "deaths_embrace";
    case 27: return "horn_of_winter";
    case 28: return "path_of_frost";
    case 29: return "resilient_grip";
    default: return 0;
    }
  case WARLOCK:
    switch ( index )
    {
    case  0: return "bane_of_agony";
    case  1: return "chaos_bolt";
    case  2: return "conflagrate";
    case  3: return "corruption";
    case  4: return "felguard";
    case  5: return "haunt";
    case  6: return "immolate";
    case  7: return "imp";
    case  8: return "incinerate";
    case  9: return "lash_of_pain";
    case 10: return "metamorphosis";
    case 11: return "shadowburn";
    case 12: return "unstable_affliction";
    case 13: return "death_coil";
    case 14: return "demonic_circle";
    case 15: return "fear";
    case 16: return "felhunter";
    case 17: return "healthstone";
    case 18: return "howl_of_terror";
    case 19: return "life_tap";
    case 20: return "seduction";
    case 21: return "shadow_bolt";
    case 22: return "shadowflame";
    case 23: return "soul_link";
    case 24: return "soul_swap";
    case 25: return "soulstone";
    case 26: return "voidwalker";
    case 27: return "curse_of_exhaustion";
    case 28: return "drain_soul";
    case 29: return "enslave_demon";
    case 30: return "eye_of_kilrogg";
    case 31: return "health_funnel";
    case 32: return "ritual_of_souls";
    case 33: return "unending_breath";
    default: return 0;
    }
  case ROGUE:
    switch ( index )
    {
    case  0: return "adrenaline_rush";
    case  1: return "backstab";
    case  2: return "eviscerate";
    case  3: return "hemorrhage";
    case  4: return "killing_spree";
    case  5: return "mutilate";
    case  6: return "revealing_strike";
    case  7: return "rupture";
    case  8: return "shadow_dance";
    case  9: return "sinister_strike";
    case 10: return "slice_and_dice";
    case 11: return "vendetta";
    case 12: return "ambush";
    case 13: return "blade_flurry";
    case 14: return "blind";
    case 15: return "cloak_of_shadows";
    case 16: return "crippling_poison";
    case 17: return "deadly_throw";
    case 18: return "evasion";
    case 19: return "expose_armor";
    case 20: return "fan_of_knives";
    case 21: return "feint";
    case 22: return "garrote";
    case 23: return "gouge";
    case 24: return "kick";
    case 25: return "preparation";
    case 26: return "sap";
    case 27: return "sprint";
    case 28: return "tricks_of_the_trade";
    case 29: return "vanish";
    case 30: return "blurred_speed";
    case 31: return "distract";
    case 32: return "pick_lock";
    case 33: return "pick_pocket";
    case 34: return "poisons";
    case 35: return "safe_fall";
    default: return 0;
    }
  case HUNTER:
    switch ( index )
    {
    case  0: return "aimed_shot";
    case  1: return "arcane_shot";
    case  2: return "chimera_shot";
    case  3: return "dazzled_prey";
    case  4: return "explosive_shot";
    case  5: return "kill_command";
    case  6: return "kill_shot";
    case  7: return "rapid_fire";
    case  8: return "serpent_sting";
    case  9: return "steady_shot";
    case 10: return "bestial_wrath";
    case 11: return "concussive_shot";
    case 12: return "deterrence";
    case 13: return "disengage";
    case 14: return "freezing_trap";
    case 15: return "ice_trap";
    case 16: return "immolation_trap";
    case 17: return "masters_call";
    case 18: return "mending";
    case 19: return "misdirection";
    case 20: return "raptor_strike";
    case 21: return "scatter_shot";
    case 22: return "silencing_shot";
    case 23: return "snake_trap";
    case 24: return "trap_launcher";
    case 25: return "wyvern_sting";
    case 26: return "aspect_of_the_pack";
    case 27: return "feign_death";
    case 28: return "lesser_proportion";
    case 29: return "revive_pet";
    case 30: return "scare_beast";
    default: return 0;
    }
  default: break;
  }

  return 0;
}

// translate_inventory_id ===================================================

static const char* translate_inventory_id( int slot )
{
  switch ( slot )
  {
  case SLOT_HEAD:      return "Head/.";
  case SLOT_NECK:      return "Neck/.";
  case SLOT_SHOULDERS: return "Shoulders/.";
  case SLOT_CHEST:     return "Chest/.";
  case SLOT_WAIST:     return "Waist/.";
  case SLOT_LEGS:      return "Legs/.";
  case SLOT_FEET:      return "Feet/.";
  case SLOT_WRISTS:    return "Wrist/.";
  case SLOT_HANDS:     return "Hands/.";
  case SLOT_FINGER_1:  return "Finger1/.";
  case SLOT_FINGER_2:  return "Finger2/.";
  case SLOT_TRINKET_1: return "Trinket1/.";
  case SLOT_TRINKET_2: return "Trinket2/.";
  case SLOT_BACK:      return "Back/.";
  case SLOT_MAIN_HAND: return "MainHand/.";
  case SLOT_OFF_HAND:  return "OffHand/.";
  case SLOT_RANGED:    return "Ranged/.";
  }

  return "unknown";
}

static race_type translate_rawr_race_str( const std::string& name )
{
  if ( ! name.compare( "Human"    ) ) return RACE_HUMAN;
  if ( ! name.compare( "Orc"      ) ) return RACE_ORC;
  if ( ! name.compare( "Dwarf"    ) ) return RACE_DWARF;
  if ( ! name.compare( "NightElf" ) ) return RACE_NIGHT_ELF;
  if ( ! name.compare( "Undead"   ) ) return RACE_UNDEAD;
  if ( ! name.compare( "Tauren"   ) ) return RACE_TAUREN;
  if ( ! name.compare( "Gnome"    ) ) return RACE_GNOME;
  if ( ! name.compare( "Troll"    ) ) return RACE_TROLL;
  if ( ! name.compare( "BloodElf" ) ) return RACE_BLOOD_ELF;
  if ( ! name.compare( "Draenei"  ) ) return RACE_DRAENEI;
  if ( ! name.compare( "Worgen"   ) ) return RACE_WORGEN;
  if ( ! name.compare( "Goblin"   ) ) return RACE_GOBLIN;

  return RACE_NONE;
}

} // ANONYMOUS NAMESPACE ====================================================

// rawr_t::load_player ======================================================

player_t* rawr_t::load_player( sim_t* sim,
                               const std::string& character_file )
{
  FILE* f = fopen( character_file.c_str(), "r" );
  if ( ! f )
  {
    sim -> errorf( "Unable to open Rawr Character Save file '%s'\n", character_file.c_str() );
    return NULL;
  }

  std::string buffer;
  char c;
  while ( ( c = fgetc( f ) ) != EOF ) buffer += c;
  fclose( f );

  player_t* p = load_player( sim, character_file, buffer );

  return p;
}

// rawr_t::load_player ======================================================

player_t* rawr_t::load_player( sim_t* sim,
                               const std::string& character_file,
                               const std::string& character_xml )
{
  xml_node_t* root_node = xml_t::create( sim, character_xml );
  if ( ! root_node )
  {
    sim -> errorf( "Unable to parse Rawr Character Save XML.\n" );
    return 0;
  }

  if ( sim -> debug ) xml_t::print( root_node );

  std::string class_str, race_str;
  if ( ! xml_t::get_value( class_str, root_node, "Class/." ) ||
       ! xml_t::get_value(  race_str, root_node, "Race/."  ) )
  {
    sim -> errorf( "Unable to determine character class and race in Rawr Character Save XML.\n" );
    return 0;
  }

  std::string name_str;
  if ( ! xml_t::get_value(  name_str, root_node, "Name/."  ) )
  {
    std::vector<std::string> tokens;
    int num_tokens = util_t::string_split( tokens, character_file, "\\/" );
    assert( num_tokens > 0 );
    name_str = tokens[ num_tokens-1 ];
  }

  sim -> current_slot = 0;
  sim -> current_name = name_str;

  std::string talents_parm = class_str + "Talents/.";

  armory_t::format(  name_str );
  armory_t::format( class_str );

  race_type r = translate_rawr_race_str( race_str );

  player_t* p = player_t::create( sim, class_str, name_str, r );
  sim -> active_player = p;
  if ( ! p )
  {
    sim -> errorf( "Unable to build player with class '%s' and name '%s'.\n", class_str.c_str(), name_str.c_str() );
    return 0;
  }

  p -> origin_str = character_file;

  xml_t::get_value( p -> region_str, root_node, "Region/." );
  xml_t::get_value( p -> server_str, root_node, "Realm/."  );

  std::string talents_str;
  if ( ! xml_t::get_value( talents_str, root_node, talents_parm ) )
  {
    sim -> errorf( "Player %s unable to determine character talents in Rawr Character Save XML.\n", p -> name() );
    return 0;
  }

  std::string talents_encoding, glyphs_encoding;
  if ( 2 != util_t::string_split( talents_str, ".", "S S", &talents_encoding, &glyphs_encoding ) )
  {
    sim -> errorf( "Player %s expected 'talents.glyphs' in Rawr Character Save XML, but found: %s\n", p -> name(), talents_str.c_str() );
    return 0;
  }

  if ( ! p -> parse_talents_armory( talents_encoding ) )
  {
    sim -> errorf( "Player %s unable to parse talent encoding '%s'.\n", p -> name(), talents_encoding.c_str() );
    return 0;
  }

  p -> talents_str = "http://www.wowhead.com/talent#";
  p -> talents_str += util_t::player_type_string( p -> type );
  p -> talents_str += "-" + talents_encoding;

  p -> glyphs_str = "";
  for ( int i=0; glyphs_encoding[ i ]; i++ )
  {
    if ( glyphs_encoding[ i ] == '1' )
    {
      const char* glyph_name = translate_glyph_name( p, i );
      if ( glyph_name )
      {
        if ( p -> glyphs_str.size() ) p -> glyphs_str += "/";
        p -> glyphs_str += glyph_name;
      }
    }
  }

  std::vector<xml_node_t*> glyph_nodes;
  int num_glyphs = xml_t::get_nodes( glyph_nodes, root_node, "Glyph" );
  for ( int i=0; i < num_glyphs; i++ )
  {
    int spell_id;
    if ( xml_t::get_value( spell_id, glyph_nodes[ i ], "." ) )
    {
      spell_data_t* sd = spell_data_t::find( spell_id );
      if ( sd )
      {
        std::string glyph_name = sd -> name_cstr();
        if (      glyph_name.substr( 0, 9 ) == "Glyph of " ) glyph_name.erase( 0, 9 );
        else if ( glyph_name.substr( 0, 8 ) == "Glyph - "  ) glyph_name.erase( 0, 8 );
        armory_t::format( glyph_name );
        if ( p -> glyphs_str.size() ) p -> glyphs_str += "/";
        p -> glyphs_str += glyph_name;
      }
      else
      {
        sim -> errorf( "Player %s unable to decode glyph id '%d'.\n", p -> name(), spell_id );
      }
    }
  }

  for ( int i=0; i < SLOT_MAX; i++ )
  {
    sim -> current_slot = i;
    if ( sim -> canceled ) return 0;

    const char* slot_name = translate_inventory_id( i );
    if ( ! slot_name ) continue;

    item_t& item = p -> items[ i ];

    std::string slot_encoding;
    if ( xml_t::get_value( slot_encoding, root_node, slot_name ) )
    {
      std::string item_id, gem_ids[ 3 ];
      std::string enchant_id, reforge_id, addon_id;
      std::string rsuffix_id;

      std::vector<std::string> splits;
      int num_splits = util_t::string_split( splits, slot_encoding, "." );

      if ( num_splits == 7 )
      {
        item_id      = splits[ 0 ];
        gem_ids[ 0 ] = splits[ 1 ];
        gem_ids[ 1 ] = splits[ 2 ];
        gem_ids[ 2 ] = splits[ 3 ];
        enchant_id   = splits[ 4 ];
        reforge_id   = splits[ 5 ];
        addon_id     = splits[ 6 ];
      }
      else if ( num_splits == 8 )
      {
        item_id      = splits[ 0 ];
        rsuffix_id   = splits[ 1 ];
        gem_ids[ 0 ] = splits[ 2 ];
        gem_ids[ 1 ] = splits[ 3 ];
        gem_ids[ 2 ] = splits[ 4 ];
        enchant_id   = splits[ 5 ];
        reforge_id   = splits[ 6 ];
        addon_id     = splits[ 7 ];
      }
      else
      {
        sim -> errorf( "Player %s unable to parse slot encoding '%s'.\n", p -> name(), slot_encoding.c_str() );
        return 0;
      }

      if ( ! item_t::download_slot( item, item_id, enchant_id, addon_id, reforge_id, rsuffix_id, gem_ids ) )
      {
        return 0;
      }
    }
  }

  // Parse Professions
  p -> professions_str = "";
  std::string profession_value[2];
  xml_t::get_value( profession_value[0], root_node, "PrimaryProfession/."   );
  xml_t::get_value( profession_value[1], root_node, "SecondaryProfession/." );
  if ( ! profession_value[0].empty() )
  {
    p -> professions_str = util_t::tolower( profession_value[0] );
    if ( ! profession_value[1].empty() )
      p -> professions_str += "/";
  }
  if ( ! profession_value[1].empty() )
    p -> professions_str += util_t::tolower( profession_value[1] );

  return p;
}
