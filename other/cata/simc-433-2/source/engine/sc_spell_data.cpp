// ==========================================================================
// Dedmonwakeen's Raid DPS/TPS Simulator.
// Send questions to natehieter@gmail.com
// ==========================================================================

#include "simulationcraft.h"

enum sdata_field_type_t
{
  SD_TYPE_INT = 0,
  SD_TYPE_UNSIGNED,
  SD_TYPE_DOUBLE,
  SD_TYPE_STR
};

struct sdata_field_t
{
  sdata_field_type_t type;
  std::string        name;
};

static const sdata_field_t _talent_data_fields[] =
{
  { SD_TYPE_STR,      "name"          },
  { SD_TYPE_UNSIGNED, "id",           },
  { SD_TYPE_UNSIGNED, "flags"         },
  { SD_TYPE_UNSIGNED, "tab"           },
  { SD_TYPE_UNSIGNED, ""              }, // Class (spell_class_expr_t)
  { SD_TYPE_UNSIGNED, ""              }, // Pet class (spell_pet_class_expr_t)
  { SD_TYPE_UNSIGNED, "dependence"    },
  { SD_TYPE_UNSIGNED, "depend_rank"   },
  { SD_TYPE_UNSIGNED, "col"           },
  { SD_TYPE_UNSIGNED, "row"           },
  { SD_TYPE_UNSIGNED, ""              }, // Talent rank spell ids, unused for now
  { SD_TYPE_UNSIGNED, ""              },
  { SD_TYPE_UNSIGNED, ""              },
};

static const sdata_field_t _effect_data_fields[] =
{
  { SD_TYPE_UNSIGNED, "id"             },
  { SD_TYPE_UNSIGNED, "flags"          },
  { SD_TYPE_UNSIGNED, "spell_id"       },
  { SD_TYPE_UNSIGNED, "index"          },
  { SD_TYPE_INT,      "type"           },
  { SD_TYPE_INT,      "sub_type"       },
  { SD_TYPE_DOUBLE,   "m_average"      },
  { SD_TYPE_DOUBLE,   "m_delta"        },
  { SD_TYPE_DOUBLE,   "m_bonus"        },
  { SD_TYPE_DOUBLE,   "coefficient"    },
  { SD_TYPE_DOUBLE,   "amplitude"      },
  { SD_TYPE_DOUBLE,   "radius"         },
  { SD_TYPE_DOUBLE,   "max_radius"     },
  { SD_TYPE_INT,      "base_value"     },
  { SD_TYPE_INT,      "misc_value"     },
  { SD_TYPE_INT,      "misc_value2"    },
  { SD_TYPE_INT,      "trigger_spell"  },
  { SD_TYPE_INT,      "m_chain"        },
  { SD_TYPE_DOUBLE,   "p_combo_points" },
  { SD_TYPE_DOUBLE,   "p_level"        },
  { SD_TYPE_INT,      "damage_range"   },
};

static const sdata_field_t _spell_data_fields[] =
{
  { SD_TYPE_STR,      "name"          },
  { SD_TYPE_UNSIGNED, "id",           },
  { SD_TYPE_UNSIGNED, "flags"         },
  { SD_TYPE_DOUBLE,   "speed"         },
  { SD_TYPE_UNSIGNED, ""              }, // School, requires custom thing
  { SD_TYPE_INT,      "power_type"    },
  { SD_TYPE_UNSIGNED, ""              }, // Class (spell_class_expr_t)
  { SD_TYPE_UNSIGNED, ""              }, // Race (spell_race_expr_t)
  { SD_TYPE_INT,      "scaling"       },
  { SD_TYPE_DOUBLE,   "extra_coeff"   },
  { SD_TYPE_UNSIGNED, "level"         },
  { SD_TYPE_UNSIGNED, "max_level"     },
  { SD_TYPE_DOUBLE,   "min_range"     },
  { SD_TYPE_DOUBLE,   "max_range"     },
  { SD_TYPE_UNSIGNED, "cooldown"      },
  { SD_TYPE_UNSIGNED, "gcd"           },
  { SD_TYPE_UNSIGNED, "category"      },
  { SD_TYPE_DOUBLE,   "duration"      },
  { SD_TYPE_UNSIGNED, "cost"          },
  { SD_TYPE_UNSIGNED, ""              }, // Runes (spell_rune_expr_t)
  { SD_TYPE_UNSIGNED, "power_gain"    },
  { SD_TYPE_UNSIGNED, "max_stack"     },
  { SD_TYPE_UNSIGNED, "proc_chance"   },
  { SD_TYPE_UNSIGNED, "initial_stack" },
  { SD_TYPE_INT,      "cast_min"      },
  { SD_TYPE_INT,      "cast_max"      },
  { SD_TYPE_INT,      "cast_div"      },
  { SD_TYPE_DOUBLE,   "m_scaling"     },
  { SD_TYPE_UNSIGNED, "scaling_level" },
  { SD_TYPE_UNSIGNED, ""              }, // Effects, 0..2, not done for now
  { SD_TYPE_UNSIGNED, ""              },
  { SD_TYPE_UNSIGNED, ""              },
  { SD_TYPE_UNSIGNED, ""              }, // Attributes, 0..9, not done for now
  { SD_TYPE_UNSIGNED, ""              },
  { SD_TYPE_UNSIGNED, ""              },
  { SD_TYPE_UNSIGNED, ""              },
  { SD_TYPE_UNSIGNED, ""              },
  { SD_TYPE_UNSIGNED, ""              },
  { SD_TYPE_UNSIGNED, ""              },
  { SD_TYPE_UNSIGNED, ""              },
  { SD_TYPE_UNSIGNED, ""              },
  { SD_TYPE_UNSIGNED, ""              },
  { SD_TYPE_STR,      "desc"          },
  { SD_TYPE_STR,      "tooltip"       },
  { SD_TYPE_STR,      ""              },
};

static const std::string _class_strings[] =
{
  "",
  "warrior",
  "paladin",
  "hunter",
  "rogue",
  "priest",
  "deathknight",
  "shaman",
  "mage",
  "monk",
  "warlock",
  "",
  "druid"
};

static const std::string _race_strings[] =
{
  "",
  "human",
  "orc",
  "dwarf",
  "night_elf",
  "undead",
  "tauren",
  "gnome",
  "troll",
  "goblin",
  "blood_elf",
  "draenei",
  "", "", "", "", "", "", "", "", "", "",
  "worgen",
};

static const std::string _pet_class_strings[] =
{
  "",
  "cunning",
  "ferocity",
  "tenacity",
};

static unsigned class_str_to_mask( const std::string& str )
{
  int cls_id = -1;

  for ( unsigned int i = 0; i < sizeof( _class_strings ) / sizeof( std::string ); i++ )
  {
    if ( _class_strings[ i ].empty() )
      continue;

    if ( ! util_t::str_compare_ci( _class_strings[ i ], str ) )
      continue;

    cls_id = i;
    break;
  }

  return 1 << ( ( cls_id < 1 ) ? 0 : cls_id - 1 );
}

static unsigned race_str_to_mask( const std::string& str )
{
  int race_id = -1;

  for ( unsigned int i = 0; i < sizeof( _race_strings ) / sizeof( std::string ); i++ )
  {
    if ( _race_strings[ i ].empty() )
      continue;

    if ( ! util_t::str_compare_ci( _race_strings[ i ], str ) )
      continue;

    race_id = i;
    break;
  }

  return 1 << ( ( race_id < 1 ) ? 0 : race_id - 1 );
}

static unsigned pet_class_str_to_mask( const std::string& str )
{
  int cls_id = -1;

  for ( unsigned int i = 0; i < sizeof( _pet_class_strings ) / sizeof( std::string ); i++ )
  {
    if ( _pet_class_strings[ i ].empty() )
      continue;

    if ( ! util_t::str_compare_ci( _pet_class_strings[ i ], str ) )
      continue;

    cls_id = i;
    break;
  }

  return 1 << ( ( cls_id < 1 ) ? 0 : cls_id - 1 );
}

static unsigned school_str_to_mask( const std::string& str )
{
  unsigned mask = 0;

  if ( util_t::str_in_str_ci( str, "physical" ) || util_t::str_in_str_ci( str, "strike" ) )
    mask |= 0x1;

  if ( util_t::str_in_str_ci( str, "holy" ) || util_t::str_in_str_ci( str, "light" ) )
    mask |= 0x2;

  if ( util_t::str_in_str_ci( str, "fire" ) || util_t::str_in_str_ci( str, "flame" ) )
    mask |= 0x4;

  if ( util_t::str_in_str_ci( str, "nature" ) || util_t::str_in_str_ci( str, "storm" ) )
    mask |= 0x8;

  if ( util_t::str_in_str_ci( str, "frost" ) )
    mask |= 0x10;

  if ( util_t::str_in_str_ci( str, "shadow" ) )
    mask |= 0x20;

  if ( util_t::str_in_str_ci( str, "arcane" ) || util_t::str_in_str_ci( str, "spell" ) )
    mask |= 0x40;

  // Special case: Arcane + Holy
  if ( util_t::str_compare_ci( "divine", str ) )
    mask = 0x42;

  return mask;
}

// Generic spell list based expression, holds intersection, union for list
// For these expression types, you can only use two spell lists as parameters
struct spell_list_expr_t : public spell_data_expr_t
{
  spell_list_expr_t( sim_t* sim, const std::string& name, expr_data_type_t type = DATA_SPELL, bool eq = false ) :
    spell_data_expr_t( sim, name, type, eq, TOK_SPELL_LIST ) { }

  virtual int evaluate()
  {
    unsigned spell_id;

    // Based on the data type, see what list of spell ids we should handle, and populate the
    // result_spell_list accordingly
    switch ( data_type )
    {
    case DATA_SPELL:
    {
      for ( const spell_data_t* spell = spell_data_t::list( sim -> dbc.ptr ); spell -> id(); spell++ )
        result_spell_list.push_back( spell -> id() );
      break;
    }
    case DATA_TALENT:
    {
      for ( const talent_data_t* talent = talent_data_t::list( sim -> dbc.ptr ); talent -> id(); talent++ )
        result_spell_list.push_back( talent -> id() );
      break;
    }
    case DATA_EFFECT:
    {
      for ( const spelleffect_data_t* effect = spelleffect_data_t::list( sim -> dbc.ptr ); effect -> id(); effect++ )
        result_spell_list.push_back( effect -> id() );
      break;
    }
    case DATA_TALENT_SPELL:
    {
      for ( const talent_data_t* talent = talent_data_t::list( sim -> dbc.ptr ); talent -> id(); talent++ )
      {
        for ( int j = 0; j < 3; j++ )
        {
          if ( ! talent -> _rank_id[ j ] )
            continue;

          result_spell_list.push_back( talent -> _rank_id[ j ] );
        }
      }
      break;
    }
    case DATA_CLASS_SPELL:
    {
      for ( unsigned cls = 0; cls < 12; cls++ )
      {
        for ( unsigned tree = 0; tree < sim -> dbc.class_ability_tree_size(); tree++ )
        {
          for ( unsigned n = 0; n < sim -> dbc.class_ability_size(); n++ )
          {
            if ( ! ( spell_id = sim -> dbc.class_ability( cls, tree, n ) ) )
              continue;

            result_spell_list.push_back( spell_id );
          }
        }
      }
      break;
    }
    case DATA_RACIAL_SPELL:
    {
      for ( unsigned race = 0; race < 24; race++ )
      {
        for ( unsigned cls = 0; cls < 12; cls++ )
        {
          for ( unsigned n = 0; n < sim -> dbc.race_ability_size(); n++ )
          {
            if ( ! ( spell_id = sim -> dbc.race_ability( race, cls, n ) ) )
              continue;

            result_spell_list.push_back( spell_id );
          }
        }
      }
      break;
    }
    case DATA_MASTERY_SPELL:
    {
      for ( unsigned cls = 0; cls < 12; cls++ )
      {
        for ( unsigned n = 0; n < sim -> dbc.mastery_ability_size(); n++ )
        {
          if ( ! ( spell_id = sim -> dbc.mastery_ability( cls, n ) ) )
            continue;

          result_spell_list.push_back( spell_id );
        }
      }
      break;
    }
    case DATA_SPECIALIZATION_SPELL:
    {
      for ( unsigned cls = 0; cls < 12; cls++ )
      {
        for ( unsigned tree = 0; tree < MAX_TALENT_TABS; tree++ )
        {
          for ( unsigned n = 0; n < sim -> dbc.specialization_ability_size(); n++ )
          {
            if ( ! ( spell_id = sim -> dbc.specialization_ability( cls, tree, n ) ) )
              continue;

            result_spell_list.push_back( spell_id );
          }
        }
      }
      break;
    }
    case DATA_GLYPH_SPELL:
    {
      for ( unsigned cls = 0; cls < 12; cls++ )
      {
        for ( unsigned type = 0; type < GLYPH_MAX; type++ )
        {
          for ( unsigned n = 0; n < sim -> dbc.glyph_spell_size(); n++ )
          {
            if ( ! ( spell_id = sim -> dbc.glyph_spell( cls, type, n ) ) )
              continue;

            result_spell_list.push_back( spell_id );
          }
        }
      }
      break;
    }
    case DATA_SET_BONUS_SPELL:
    {
      for ( unsigned cls = 0; cls < 12; cls++ )
      {
        for ( unsigned tier = 0; tier < N_TIER; tier++ )
        {
          for ( unsigned n = 0; n < sim -> dbc.set_bonus_spell_size(); n++ )
          {
            if ( ! ( spell_id = sim -> dbc.set_bonus_spell( cls, tier, n ) ) )
              continue;

            result_spell_list.push_back( spell_id );
          }
        }
      }
      break;
    }
    default:
      return TOK_UNKNOWN;
    }

    result_spell_list.resize( range::unique( range::sort( result_spell_list ) ) - result_spell_list.begin() );

    return TOK_SPELL_LIST;
  }

  // Intersect two spell lists
  virtual std::vector<uint32_t> operator&( const spell_data_expr_t& other )
  {
    std::vector<uint32_t> res;

    // Only or two spell lists together
    if ( other.result_type != TOK_SPELL_LIST )
    {
      sim -> errorf( "Unsupported right side operand '%s' (%d) for operator &",
                     other.name_str.c_str(),
                     other.result_type );
    }
    else
      range::set_intersection( result_spell_list, other.result_spell_list, std::back_inserter( res ) );

    return res;
  }

  // Merge two spell lists, uniqueing entries
  virtual std::vector<uint32_t> operator|( const spell_data_expr_t& other )
  {
    std::vector<uint32_t> res;

    // Only or two spell lists together
    if ( other.result_type != TOK_SPELL_LIST )
    {
      sim -> errorf( "Unsupported right side operand '%s' (%d) for operator |",
                     other.name_str.c_str(),
                     other.result_type );
    }
    else
      range::set_union( result_spell_list, other.result_spell_list, std::back_inserter( res ) );

    return res;
  }

  // Subtract two spell lists, other from this
  virtual std::vector<uint32_t>operator-( const spell_data_expr_t& other )
  {
    std::vector<uint32_t> res;

    // Only or two spell lists together
    if ( other.result_type != TOK_SPELL_LIST )
    {
      sim -> errorf( "Unsupported right side operand '%s' (%d) for operator -",
                     other.name_str.c_str(),
                     other.result_type );
    }
    else
      range::set_difference( result_spell_list, other.result_spell_list, std::back_inserter( res ) );

    return res;
  }
};

struct sd_expr_binary_t : public spell_list_expr_t
{
  int                operation;
  spell_data_expr_t* left;
  spell_data_expr_t* right;

  sd_expr_binary_t( sim_t* sim, const std::string& n, int o, spell_data_expr_t* l, spell_data_expr_t* r ) :
    spell_list_expr_t( sim, n ), operation( o ), left( l ), right( r ) { }

  virtual int evaluate()
  {
    int  left_result =  left -> evaluate();

    right -> evaluate();
    result_type      = TOK_UNKNOWN;

    if ( left_result != TOK_SPELL_LIST )
    {
      sim -> errorf( "Inconsistent input types (%s and %s) for binary operator '%s', left must always be a spell list.\n",
                     left -> name(), right -> name(), name() );
      sim -> cancel();
    }
    else
    {
      result_type = TOK_SPELL_LIST;
      // Data type follows from left side operand
      data_type   = left -> data_type;

      switch ( operation )
      {
      case TOK_EQ:    result_spell_list = *left == *right; break;
      case TOK_NOTEQ: result_spell_list = *left != *right; break;
      case TOK_OR:    result_spell_list = *left | *right; break;
      case TOK_AND:   result_spell_list = *left & *right; break;
      case TOK_SUB:   result_spell_list = *left - *right; break;
      case TOK_LT:    result_spell_list = *left < *right; break;
      case TOK_LTEQ:  result_spell_list = *left <= *right; break;
      case TOK_GT:    result_spell_list = *left > *right; break;
      case TOK_GTEQ:  result_spell_list = *left >= *right; break;
      case TOK_IN:    result_spell_list = left -> in( *right ); break;
      case TOK_NOTIN: result_spell_list = left -> not_in( *right ); break;
      default:
        sim -> errorf( "Unsupported spell query operator %d", operation );
        result_spell_list = std::vector<uint32_t>();
        result_type = TOK_UNKNOWN;
        break;
      }
    }

    return result_type;
  }
};

struct spell_data_filter_expr_t : public spell_list_expr_t
{
  int                offset;
  sdata_field_type_t field_type;

  spell_data_filter_expr_t( sim_t* sim, expr_data_type_t type, const std::string& f_name, bool eq = false ) :
    spell_list_expr_t( sim, f_name, type, eq ), offset( 0 ), field_type( SD_TYPE_INT )
  {
    const sdata_field_t      * fields = 0;
    unsigned             fsize;
    if ( type == DATA_TALENT )
    {
      fields = _talent_data_fields;
      fsize  = sizeof( _talent_data_fields );
    }
    else if ( effect_query || type == DATA_EFFECT )
    {
      fields = _effect_data_fields;
      fsize  = sizeof( _effect_data_fields );
    }
    else
    {
      fields = _spell_data_fields;
      fsize  = sizeof( _spell_data_fields );
    }

    // Figure out our offset then
    for ( unsigned int i = 0; i < fsize / sizeof( sdata_field_t ); i++ )
    {
      if ( fields[ i ].name.empty() || ! util_t::str_compare_ci( f_name, fields[ i ].name ) )
      {
        switch ( fields[ i ].type )
        {
        case SD_TYPE_INT:
        case SD_TYPE_UNSIGNED:
          offset += sizeof( int );
          break;
        case SD_TYPE_DOUBLE:
          offset += sizeof( double );
          break;
        case SD_TYPE_STR:
          offset += sizeof( const char* );
          break;
        default:
          sim -> errorf( "Unknown field type %d for %s.",
                         fields[ i ].type,
                         fields[ i ].name.c_str() );
          break;
        }

        continue;
      }

      field_type = fields[ i ].type;
      break;
    }
  }

  virtual bool compare( char* data, const spell_data_expr_t& other, token_type_t t ) const
  {
    const int      *int_v;
    int             oint_v;
    const double   *double_v;
    const unsigned *unsigned_v;
    unsigned        ounsigned_v;
    std::string     string_v,
    ostring_v;

    switch ( field_type )
    {
    case SD_TYPE_INT:
    {
      int_v  = reinterpret_cast< const int* >( data + offset );
      oint_v = ( int ) other.result_num;
      switch ( t )
      {
      case TOK_LT:     if ( *int_v < oint_v  ) return true; break;
      case TOK_LTEQ:   if ( *int_v <= oint_v ) return true; break;
      case TOK_GT:     if ( *int_v > oint_v  ) return true; break;
      case TOK_GTEQ:   if ( *int_v >= oint_v ) return true; break;
      case TOK_EQ:     if ( *int_v == oint_v ) return true; break;
      case TOK_NOTEQ:  if ( *int_v != oint_v ) return true; break;
      default:         return false;
      }
      break;
    }
    case SD_TYPE_UNSIGNED:
    {
      unsigned_v  = reinterpret_cast< const unsigned* >( data + offset );
      ounsigned_v = ( unsigned ) other.result_num;
      switch ( t )
      {
      case TOK_LT:     if ( *unsigned_v < ounsigned_v ) return true; break;
      case TOK_LTEQ:   if ( *unsigned_v <= ounsigned_v ) return true; break;
      case TOK_GT:     if ( *unsigned_v > ounsigned_v ) return true; break;
      case TOK_GTEQ:   if ( *unsigned_v >= ounsigned_v ) return true; break;
      case TOK_EQ:     if ( *unsigned_v == ounsigned_v ) return true; break;
      case TOK_NOTEQ:  if ( *unsigned_v != ounsigned_v ) return true; break;
      default:         return false;
      }
      break;
    }
    case SD_TYPE_DOUBLE:
    {
      double_v  = reinterpret_cast< const double* >( data + offset );
      switch ( t )
      {
      case TOK_LT:     if ( *double_v < other.result_num ) return true; break;
      case TOK_LTEQ:   if ( *double_v <= other.result_num ) return true; break;
      case TOK_GT:     if ( *double_v > other.result_num ) return true; break;
      case TOK_GTEQ:   if ( *double_v >= other.result_num ) return true; break;
      case TOK_EQ:     if ( *double_v == other.result_num ) return true; break;
      case TOK_NOTEQ:  if ( *double_v != other.result_num ) return true; break;
      default:         return false;
      }
      break;
    }
    case SD_TYPE_STR:
    {
      if ( *reinterpret_cast<const char**>( data + offset ) )
        string_v = std::string( *reinterpret_cast<const char**>( data + offset ) );
      else
        string_v = "";
      armory_t::format( string_v );
      ostring_v = other.result_str;

      switch ( t )
      {
      case TOK_EQ:    if ( util_t::str_compare_ci( string_v, ostring_v ) ) return true; break;
      case TOK_NOTEQ: if ( ! util_t::str_compare_ci( string_v, ostring_v ) ) return true; break;
      case TOK_IN:    if ( ! string_v.empty() && util_t::str_in_str_ci( string_v, ostring_v ) ) return true; break;
      case TOK_NOTIN: if ( ! string_v.empty() && ! util_t::str_in_str_ci( string_v, ostring_v ) ) return true; break;
      default:        return false;
      }
      break;
    }
    default:
    {
      break;
    }
    }
    return false;
  }

  void build_list( std::vector<uint32_t>& res, const spell_data_expr_t& other, token_type_t t ) const
  {
    char* p_data = 0;

    for ( std::vector<uint32_t>::const_iterator i = result_spell_list.begin(); i != result_spell_list.end(); i++ )
    {
      if ( effect_query )
      {
        for ( int j = 0; j < MAX_EFFECTS; j++ )
        {
          const spell_data_t* spell = sim -> dbc.spell( *i );
          if ( spell && spell -> _effect[ j ] &&
               sim -> dbc.effect( spell -> _effect[ j ] ) )
            p_data = reinterpret_cast< char* > ( const_cast< spelleffect_data_t* >( sim -> dbc.effect( spell -> _effect[ j ] ) ) );
          else
            p_data = 0;

          if ( p_data && range::find( res, *i ) == res.end() && compare( p_data, other, t ) )
            res.push_back( *i );
        }
      }
      else
      {
        if ( data_type == DATA_TALENT )
          p_data = reinterpret_cast< char* > ( const_cast< talent_data_t* >( sim -> dbc.talent( *i ) ) );
        else if ( data_type == DATA_EFFECT )
          p_data = reinterpret_cast< char* > ( const_cast< spelleffect_data_t* >( sim -> dbc.effect( *i ) ) );
        else
          p_data = reinterpret_cast< char* > ( const_cast< spell_data_t* >( sim -> dbc.spell( *i ) ) );

        if ( p_data && range::find( res, *i ) == res.end() && compare( p_data, other, t ) )
          res.push_back( *i );
      }
    }
  }

  virtual std::vector<uint32_t> operator==( const spell_data_expr_t& other )
  {
    std::vector<uint32_t> res;

    if ( other.result_type != TOK_NUM && other.result_type != TOK_STR )
    {
      sim -> errorf( "Unsupported expression operator == for left=%s(%d), right=%s(%d)",
                     name_str.c_str(),
                     result_type,
                     other.name_str.c_str(),
                     other.result_type );
    }
    else
      build_list( res, other, TOK_EQ );

    return res;
  }

  virtual std::vector<uint32_t> operator!=( const spell_data_expr_t& other )
  {
    std::vector<uint32_t> res;

    if ( other.result_type != TOK_NUM && other.result_type != TOK_STR )
    {
      sim -> errorf( "Unsupported expression operator != for left=%s(%d), right=%s(%d)",
                     name_str.c_str(),
                     result_type,
                     other.name_str.c_str(),
                     other.result_type );
    }
    else
      build_list( res, other, TOK_NOTEQ );

    return res;
  }

  virtual std::vector<uint32_t> operator<( const spell_data_expr_t& other )
  {
    std::vector<uint32_t> res;

    if ( other.result_type != TOK_NUM ||
         ( field_type != SD_TYPE_INT && field_type != SD_TYPE_UNSIGNED && field_type != SD_TYPE_DOUBLE )  )
    {
      sim -> errorf( "Unsupported expression operator < for left=%s(%d), right=%s(%d) or field '%s' is not a number",
                     name_str.c_str(),
                     result_type,
                     other.name_str.c_str(),
                     other.result_type,
                     name_str.c_str() );
    }
    else
      build_list( res, other, TOK_LT );

    return res;
  }

  virtual std::vector<uint32_t> operator<=( const spell_data_expr_t& other )
  {
    std::vector<uint32_t> res;

    if ( other.result_type != TOK_NUM ||
         ( field_type != SD_TYPE_INT && field_type != SD_TYPE_UNSIGNED && field_type != SD_TYPE_DOUBLE )  )
    {
      sim -> errorf( "Unsupported expression operator <= for left=%s(%d), right=%s(%d) or field '%s' is not a number",
                     name_str.c_str(),
                     result_type,
                     other.name_str.c_str(),
                     other.result_type,
                     name_str.c_str() );
    }
    else
      build_list( res, other, TOK_LTEQ );

    return res;
  }

  virtual std::vector<uint32_t> operator>( const spell_data_expr_t& other )
  {
    std::vector<uint32_t> res;

    if ( other.result_type != TOK_NUM ||
         ( field_type != SD_TYPE_INT && field_type != SD_TYPE_UNSIGNED && field_type != SD_TYPE_DOUBLE )  )
    {
      sim -> errorf( "Unsupported expression operator > for left=%s(%d), right=%s(%d) or field '%s' is not a number",
                     name_str.c_str(),
                     result_type,
                     other.name_str.c_str(),
                     other.result_type,
                     name_str.c_str() );
    }
    else
      build_list( res, other, TOK_GT );

    return res;
  }

  virtual std::vector<uint32_t> operator>=( const spell_data_expr_t& other )
  {
    std::vector<uint32_t> res;

    if ( other.result_type != TOK_NUM ||
         ( field_type != SD_TYPE_INT && field_type != SD_TYPE_UNSIGNED && field_type != SD_TYPE_DOUBLE )  )
    {
      sim -> errorf( "Unsupported expression operator >= for left=%s(%d), right=%s(%d) or field '%s' is not a number",
                     name_str.c_str(),
                     result_type,
                     other.name_str.c_str(),
                     other.result_type,
                     name_str.c_str() );
    }
    else
      build_list( res, other, TOK_GTEQ );

    return res;
  }

  virtual std::vector<uint32_t> in( const spell_data_expr_t& other )
  {
    std::vector<uint32_t> res;

    if ( other.result_type != TOK_STR || field_type != SD_TYPE_STR )
    {
      sim -> errorf( "Unsupported expression operator ~ for left=%s(%d), right=%s(%d) or field '%s' is not a string",
                     name_str.c_str(),
                     result_type,
                     other.name_str.c_str(),
                     other.result_type,
                     name_str.c_str() );
    }
    else
      build_list( res, other, TOK_IN );

    return res;
  }

  virtual std::vector<uint32_t> not_in( const spell_data_expr_t& other )
  {
    std::vector<uint32_t> res;

    if ( other.result_type != TOK_STR || field_type != SD_TYPE_STR )
    {
      sim -> errorf( "Unsupported expression operator !~ for left=%s(%d), right=%s(%d) or field '%s' is not a string",
                     name_str.c_str(),
                     result_type,
                     other.name_str.c_str(),
                     other.result_type,
                     name_str.c_str() );
    }
    else
      build_list( res, other, TOK_NOTIN );

    return res;
  }
};

struct spell_rune_expr_t : public spell_list_expr_t
{
  static unsigned rune_cost( const std::string& s )
  {
    const char _runes[] = { 'b', 'u', 'f' };
    int n_runes[]       = { 0, 0, 0 };
    unsigned rune_mask  = 0;

    for ( unsigned int i = 0; i < s.size(); i++ )
    {
      for ( unsigned int j = 0; j < 3; j++ )
      {
        if ( s[ i ] == _runes[ j ] )
          n_runes[ j ]++;
      }
    }

    for ( unsigned int i = 0; i < 3; i++ )
    {
      for ( int j = 0; j < std::min( 2, n_runes[ i ] ); j++ )
        rune_mask |= ( 1 << ( i * 2 + j ) );
    }

    return rune_mask;
  }

  spell_rune_expr_t( sim_t* sim, expr_data_type_t type ) : spell_list_expr_t( sim, "rune", type ) { }

  virtual std::vector<uint32_t> operator==( const spell_data_expr_t& other )
  {
    std::vector<uint32_t> res;
    unsigned              r = rune_cost( other.result_str );

    for ( std::vector<uint32_t>::const_iterator i = result_spell_list.begin(); i != result_spell_list.end(); i++ )
    {
      const spell_data_t* spell = sim -> dbc.spell( *i );
      if ( ! spell || spell -> power_type() != 5 )
        continue;

      if ( ( spell -> rune_cost() & r ) == r )
        res.push_back( *i );
    }

    return res;
  }

  virtual std::vector<uint32_t> operator!=( const spell_data_expr_t& other )
  {
    std::vector<uint32_t> res;
    unsigned              r = rune_cost( other.result_str );

    for ( std::vector<uint32_t>::const_iterator i = result_spell_list.begin(); i != result_spell_list.end(); i++ )
    {
      const spell_data_t* spell = sim -> dbc.spell( *i );
      if ( ! spell || spell -> power_type() != 5 )
        continue;

      if ( ( spell -> rune_cost() & r ) != r )
        res.push_back( *i );
    }

    return res;
  }
};

struct spell_class_expr_t : public spell_list_expr_t
{
  spell_class_expr_t( sim_t* sim, expr_data_type_t type ) : spell_list_expr_t( sim, "class", type ) { }

  virtual std::vector<uint32_t> operator==( const spell_data_expr_t& other )
  {
    std::vector<uint32_t> res;
    uint32_t              class_mask;

    if ( other.result_type == TOK_STR )
      class_mask = class_str_to_mask( other.result_str );
    // Other types will not be allowed, e.g. you cannot do class=list
    else
      return res;

    for ( std::vector<uint32_t>::const_iterator i = result_spell_list.begin(); i != result_spell_list.end(); i++ )
    {
      if ( data_type == DATA_TALENT )
      {
        if ( ! sim -> dbc.talent( *i ) )
          continue;

        if ( sim -> dbc.talent( *i ) -> mask_class() & class_mask )
          res.push_back( *i );
      }
      else
      {
        const spell_data_t* spell = sim -> dbc.spell( *i );

        if ( ! spell )
          continue;

        if ( spell -> class_mask() & class_mask )
          res.push_back( *i );
      }
    }

    return res;
  }

  virtual std::vector<uint32_t> operator!=( const spell_data_expr_t& other )
  {
    std::vector<uint32_t> res;
    uint32_t              class_mask;

    if ( other.result_type == TOK_STR )
      class_mask = class_str_to_mask( other.result_str );
    // Other types will not be allowed, e.g. you cannot do class=list
    else
      return res;

    for ( std::vector<uint32_t>::const_iterator i = result_spell_list.begin(); i != result_spell_list.end(); i++ )
    {
      if ( data_type == DATA_TALENT )
      {
        if ( ! sim -> dbc.talent( *i ) )
          continue;

        if ( ( sim -> dbc.talent( *i ) -> mask_class() & class_mask ) == 0 )
          res.push_back( *i );
      }
      else
      {
        const spell_data_t* spell = sim -> dbc.spell( *i );
        if ( ! spell )
          continue;

        if ( ( spell -> class_mask() & class_mask ) == 0 )
          res.push_back( *i );
      }
    }

    return res;
  }
};

struct spell_pet_class_expr_t : public spell_list_expr_t
{
  spell_pet_class_expr_t( sim_t* sim, expr_data_type_t type ) : spell_list_expr_t( sim, "pet_class", type ) { }

  virtual std::vector<uint32_t> operator==( const spell_data_expr_t& other )
  {
    std::vector<uint32_t> res;
    uint32_t              class_mask;

    // Pet class is only stored in talent data
    if ( data_type != DATA_TALENT )
      return res;

    if ( other.result_type == TOK_STR )
      class_mask = pet_class_str_to_mask( other.result_str );
    // Other types will not be allowed, e.g. you cannot do class=list
    else
      return res;

    for ( std::vector<uint32_t>::const_iterator i = result_spell_list.begin(); i != result_spell_list.end(); i++ )
    {
      if ( ! sim -> dbc.talent( *i ) )
        continue;

      if ( sim -> dbc.talent( *i ) -> mask_pet() & class_mask )
        res.push_back( *i );
    }

    return res;
  }

  virtual std::vector<uint32_t> operator!=( const spell_data_expr_t& other )
  {
    std::vector<uint32_t> res;
    uint32_t              class_mask;

    if ( other.result_type == TOK_STR )
      class_mask = pet_class_str_to_mask( other.result_str );
    // Other types will not be allowed, e.g. you cannot do class=list
    else
      return res;

    for ( std::vector<uint32_t>::const_iterator i = result_spell_list.begin(); i != result_spell_list.end(); i++ )
    {
      if ( ! sim -> dbc.talent( *i ) )
        continue;

      if ( ( sim -> dbc.talent( *i ) -> mask_pet() & class_mask ) == 0 )
        res.push_back( *i );
    }

    return res;
  }
};

struct spell_race_expr_t : public spell_list_expr_t
{
  spell_race_expr_t( sim_t* sim, expr_data_type_t type ) : spell_list_expr_t( sim, "race", type ) { }

  virtual std::vector<uint32_t> operator==( const spell_data_expr_t& other )
  {
    std::vector<uint32_t> res;
    uint32_t              race_mask;

    // Talents are not race specific
    if ( data_type == DATA_TALENT )
      return res;

    if ( other.result_type == TOK_STR )
      race_mask = race_str_to_mask( other.result_str );
    // Other types will not be allowed, e.g. you cannot do race=list
    else
      return res;

    for ( std::vector<uint32_t>::const_iterator i = result_spell_list.begin(); i != result_spell_list.end(); i++ )
    {
      const spell_data_t* spell = sim -> dbc.spell( *i );

      if ( ! spell )
        continue;

      if ( spell -> race_mask() & race_mask )
        res.push_back( *i );
    }

    return res;
  }

  virtual std::vector<uint32_t> operator!=( const spell_data_expr_t& other )
  {
    std::vector<uint32_t> res;
    uint32_t              class_mask;

    // Talents are not race specific
    if ( data_type == DATA_TALENT )
      return res;

    if ( other.result_type == TOK_STR )
      class_mask = race_str_to_mask( other.result_str );
    // Other types will not be allowed, e.g. you cannot do class=list
    else
      return res;

    for ( std::vector<uint32_t>::const_iterator i = result_spell_list.begin(); i != result_spell_list.end(); i++ )
    {
      const spell_data_t* spell = sim -> dbc.spell( *i );
      if ( ! spell )
        continue;

      if ( ( spell -> class_mask() & class_mask ) == 0 )
        res.push_back( *i );
    }

    return res;
  }
};

struct spell_attribute_expr_t : public spell_list_expr_t
{
  spell_attribute_expr_t( sim_t* sim, expr_data_type_t type ) : spell_list_expr_t( sim, "attribute", type ) { }

  virtual std::vector<uint32_t> operator==( const spell_data_expr_t& other )
  {
    std::vector<uint32_t> res;

    // Only for spells
    if ( data_type == DATA_EFFECT || data_type == DATA_TALENT )
      return res;

    // Numbered attributes only
    if ( other.result_type != TOK_NUM )
      return res;

    uint32_t attridx = ( unsigned ) other.result_num / ( sizeof( unsigned ) * 8 );
    uint32_t flagidx = ( unsigned ) other.result_num % ( sizeof( unsigned ) * 8 );

    assert( attridx < 10 && flagidx < 32 );

    for ( std::vector<uint32_t>::const_iterator i = result_spell_list.begin(); i != result_spell_list.end(); i++ )
    {
      const spell_data_t* spell = sim -> dbc.spell( *i );

      if ( ! spell )
        continue;

      if ( spell -> _attributes[ attridx ] & ( 1 << flagidx ) )
        res.push_back( *i );
    }

    return res;
  }
};

struct spell_school_expr_t : public spell_list_expr_t
{
  spell_school_expr_t( sim_t* sim, expr_data_type_t type ) : spell_list_expr_t( sim, "school", type ) { }

  virtual std::vector<uint32_t> operator==( const spell_data_expr_t& other )
  {
    std::vector<uint32_t> res;
    uint32_t              school_mask;

    if ( other.result_type == TOK_STR )
      school_mask = school_str_to_mask( other.result_str );
    // Other types will not be allowed, e.g. you cannot do class=list
    else
      return res;

    for ( std::vector<uint32_t>::const_iterator i = result_spell_list.begin(); i != result_spell_list.end(); i++ )
    {
      const spell_data_t* spell = sim -> dbc.spell( *i );

      if ( ! spell )
        continue;

      if ( ( spell -> school_mask() & school_mask ) == school_mask )
        res.push_back( *i );
    }

    return res;
  }

  virtual std::vector<uint32_t> operator!=( const spell_data_expr_t& other )
  {
    std::vector<uint32_t> res;
    uint32_t              school_mask;

    if ( other.result_type == TOK_STR )
      school_mask = school_str_to_mask( other.result_str );
    // Other types will not be allowed, e.g. you cannot do school=list
    else
      return res;

    for ( std::vector<uint32_t>::const_iterator i = result_spell_list.begin(); i != result_spell_list.end(); i++ )
    {
      const spell_data_t* spell = sim -> dbc.spell( *i );

      if ( ! spell )
        continue;

      if ( ( spell -> school_mask() & school_mask ) == 0 )
        res.push_back( *i );
    }

    return res;
  }
};

template <typename Container, typename Disposer=delete_disposer_t>
class auto_dispose_t : public Container
{
private:
  template <typename D>
  void dispose_( D disposer )
  { range::dispose( *this, disposer ); }

  void dispose_()
  { dispose_( Disposer() ); }

public:
  ~auto_dispose_t() { dispose_(); }

  using Container::clear;

  void dispose()
  { dispose_(); clear(); }

  template <typename D>
  void dispose( D disposer )
  { dispose_( disposer ); clear(); }
};

static spell_data_expr_t* build_expression_tree( sim_t* sim,
                                                 const std::vector<expr_token_t>& tokens )
{
  auto_dispose_t< std::vector<spell_data_expr_t*> > stack;

  size_t num_tokens = tokens.size();
  for ( size_t i=0; i < num_tokens; i++ )
  {
    const expr_token_t& t= tokens[ i ];

    if ( t.type == TOK_NUM )
      stack.push_back( new spell_data_expr_t( sim, t.label, atof( t.label.c_str() ) ) );
    else if ( t.type == TOK_STR )
    {
      spell_data_expr_t* e = spell_data_expr_t::create_spell_expression( sim, t.label );

      if ( ! e )
      {
        sim -> errorf( "Unable to decode expression function '%s'\n", t.label.c_str() );
        return 0;
      }
      stack.push_back( e );
    }
    else if ( expression_t::is_binary( t.type ) )
    {
      if ( stack.size() < 2 )
        return 0;
      spell_data_expr_t* right = stack.back(); stack.pop_back();
      spell_data_expr_t* left  = stack.back(); stack.pop_back();
      if ( ! left || ! right )
        return 0;
      stack.push_back( new sd_expr_binary_t( sim, t.label, t.type, left, right ) );
    }
  }

  spell_data_expr_t* res = 0;
  if ( stack.size() == 1 )
  {
    res = stack.back();
    stack.pop_back();
  }
  return res;
}

spell_data_expr_t* spell_data_expr_t::create_spell_expression( sim_t* sim, const std::string& name_str )
{
  std::vector<std::string> splits;
  std::string              v;
  bool                     effect_query = false;
  int                      num_splits = util_t::string_split( splits, name_str, "." );
  expr_data_type_t         data_type = DATA_SPELL;

  if ( num_splits < 1 || num_splits > 3 )
    return 0;

  // No split, access raw list or create a normal expression
  if ( num_splits == 1 )
  {
    if ( util_t::str_compare_ci( splits[ 0 ], "spell" ) )
      return new spell_list_expr_t( sim, splits[ 0 ], DATA_SPELL );
    else if ( util_t::str_compare_ci( splits[ 0 ], "talent" ) )
      return new spell_list_expr_t( sim, splits[ 0 ], DATA_TALENT );
    else if ( util_t::str_compare_ci( splits[ 0 ], "effect" ) )
      return new spell_list_expr_t( sim, splits[ 0 ], DATA_EFFECT );
    else if ( util_t::str_compare_ci( splits[ 0 ], "talent_spell" ) )
      return new spell_list_expr_t( sim, splits[ 0 ], DATA_TALENT_SPELL );
    else if ( util_t::str_compare_ci( splits[ 0 ], "class_spell" ) )
      return new spell_list_expr_t( sim, splits[ 0 ], DATA_CLASS_SPELL );
    else if ( util_t::str_compare_ci( splits[ 0 ], "race_spell" ) )
      return new spell_list_expr_t( sim, splits[ 0 ], DATA_RACIAL_SPELL );
    else if ( util_t::str_compare_ci( splits[ 0 ], "mastery" ) )
      return new spell_list_expr_t( sim, splits[ 0 ], DATA_MASTERY_SPELL );
    else if ( util_t::str_compare_ci( splits[ 0 ], "spec_spell" ) )
      return new spell_list_expr_t( sim, splits[ 0 ], DATA_SPECIALIZATION_SPELL );
    else if ( util_t::str_compare_ci( splits[ 0 ], "glyph" ) )
      return new spell_list_expr_t( sim, splits[ 0 ], DATA_GLYPH_SPELL );
    else if ( util_t::str_compare_ci( splits[ 0 ], "set_bonus" ) )
      return new spell_list_expr_t( sim, splits[ 0 ], DATA_SET_BONUS_SPELL );
    else
    {
      v = name_str;
      return new spell_data_expr_t( sim, name_str, v );
    }
  }
  else // Define data type
  {
    if ( util_t::str_compare_ci( splits[ 0 ], "spell" ) )
      data_type = DATA_SPELL;
    else if ( util_t::str_compare_ci( splits[ 0 ], "talent" ) )
      data_type = DATA_TALENT;
    else if ( util_t::str_compare_ci( splits[ 0 ], "effect" ) )
      data_type = DATA_EFFECT;
    else if ( util_t::str_compare_ci( splits[ 0 ], "talent_spell" ) )
      data_type = DATA_TALENT_SPELL;
    else if ( util_t::str_compare_ci( splits[ 0 ], "class_spell" ) )
      data_type = DATA_CLASS_SPELL;
    else if ( util_t::str_compare_ci( splits[ 0 ], "race_spell" ) )
      data_type = DATA_RACIAL_SPELL;
    else if ( util_t::str_compare_ci( splits[ 0 ], "mastery" ) )
      data_type = DATA_MASTERY_SPELL;
    else if ( util_t::str_compare_ci( splits[ 0 ], "spec_spell" ) )
      data_type = DATA_SPECIALIZATION_SPELL;
    else if ( util_t::str_compare_ci( splits[ 0 ], "glyph" ) )
      data_type = DATA_GLYPH_SPELL;
    else if ( util_t::str_compare_ci( splits[ 0 ], "set_bonus" ) )
      data_type = DATA_SET_BONUS_SPELL;

    if ( util_t::str_compare_ci( splits[ 1 ], "effect" ) && data_type == DATA_EFFECT )
      return 0;

    // Effect handling, set flag and remove effect keyword from tokens
    if ( util_t::str_compare_ci( splits[ 1 ], "effect" ) )
    {
      effect_query = true;
      splits.erase( splits.begin() + 1 );
    }

    if ( ! effect_query && util_t::str_compare_ci( splits[ 1 ], "class" ) )
      return new spell_class_expr_t( sim, data_type );
    else if ( ! effect_query && util_t::str_compare_ci( splits[ 1 ], "race" ) )
      return new spell_race_expr_t( sim, data_type );
    else if ( ! effect_query && util_t::str_compare_ci( splits[ 1 ], "attribute" ) )
      return new spell_attribute_expr_t( sim, data_type );
    else if ( ! effect_query && data_type == DATA_TALENT && util_t::str_compare_ci( splits[ 1 ], "pet_class" ) )
      return new spell_pet_class_expr_t( sim, data_type );
    else if ( ! effect_query && data_type != DATA_TALENT && util_t::str_compare_ci( splits[ 1 ], "school" ) )
      return new spell_school_expr_t( sim, data_type );
    else if ( ! effect_query && data_type != DATA_TALENT && util_t::str_compare_ci( splits[ 1 ], "rune" ) )
      return new spell_rune_expr_t( sim, data_type );
    else
    {
      const sdata_field_t* s = 0;
      const sdata_field_t* fields = 0;
      unsigned       fsize;
      if ( data_type == DATA_TALENT )
      {
        fields = _talent_data_fields;
        fsize  = sizeof( _talent_data_fields );
      }
      else if ( effect_query || data_type == DATA_EFFECT )
      {
        fields = _effect_data_fields;
        fsize  = sizeof( _effect_data_fields );
      }
      else
      {
        fields = _spell_data_fields;
        fsize  = sizeof( _spell_data_fields );
      }

      for ( unsigned int i = 0; i < fsize / sizeof( sdata_field_t ); i++ )
      {
        if ( ! fields[ i ].name.empty() && util_t::str_compare_ci( splits[ 1 ], fields[ i ].name ) )
        {
          s = &fields[ i ];
          break;
        }
      }

      if ( s )
        return new spell_data_filter_expr_t( sim, data_type, s -> name, effect_query );
      else
        return 0;
    }
  }

  return 0;
}

spell_data_expr_t* spell_data_expr_t::parse( sim_t* sim, const std::string& expr_str )
{
  if ( expr_str.empty() ) return 0;

  std::vector<expr_token_t> tokens;

  expression_t::parse_tokens( 0, tokens, expr_str );

  if ( sim -> debug ) expression_t::print_tokens( tokens, sim );

  expression_t::convert_to_unary( 0, tokens );

  if ( sim -> debug ) expression_t::print_tokens( tokens, sim );

  if ( ! expression_t::convert_to_rpn( 0, tokens ) )
  {
    sim -> errorf( "Unable to convert %s into RPN\n", expr_str.c_str() );
    sim -> cancel();
    return 0;
  }

  if ( sim -> debug ) expression_t::print_tokens( tokens, sim );

  spell_data_expr_t* e = build_expression_tree( sim, tokens );

  if ( ! e )
  {
    sim -> errorf( "Unable to build expression tree from %s\n", expr_str.c_str() );
    sim -> cancel();
    return 0;
  }

  return e;
}
