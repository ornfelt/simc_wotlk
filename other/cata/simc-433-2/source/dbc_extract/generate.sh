#!/bin/sh
if [ $# -lt 1 ]; then
    echo "Usage: generate.sh [ptr] <patch> <input_base>"
    exit 1
fi

OUTPATH=`dirname $PWD`/engine
INEXT=DBFilesClient

PTR=
if [ "$1" == "ptr" ]; then
    PTR=" --prefix=ptr"
    shift
fi

BUILD=$1
INPUT=${2}/${1}/${INEXT}

if [ ! -d $INPUT ]; then
  echo Error: Unable to find input files.
  echo "Usage: generate.sh [ptr] <patch> <input_base>"
  exit 1
fi

./dbc_extract.py -p $INPUT -b $BUILD$PTR -t talent                  > $OUTPATH/sc_talent_data${PTR:+_ptr}.inc
./dbc_extract.py -p $INPUT --itemcache=$INPUT -b $BUILD$PTR -t spell> $OUTPATH/sc_spell_data${PTR:+_ptr}.inc
./dbc_extract.py -p $INPUT -b $BUILD$PTR -t scale                   > $OUTPATH/sc_scale_data${PTR:+_ptr}.inc
./dbc_extract.py -p $INPUT -b $BUILD$PTR -t class_list              > $OUTPATH/sc_spell_lists${PTR:+_ptr}.inc
./dbc_extract.py -p $INPUT -b $BUILD$PTR -t spec_spell_list        >> $OUTPATH/sc_spell_lists${PTR:+_ptr}.inc
./dbc_extract.py -p $INPUT -b $BUILD$PTR -t mastery_list           >> $OUTPATH/sc_spell_lists${PTR:+_ptr}.inc
./dbc_extract.py -p $INPUT -b $BUILD$PTR -t racial_list            >> $OUTPATH/sc_spell_lists${PTR:+_ptr}.inc
./dbc_extract.py -p $INPUT -b $BUILD$PTR -t glyph_list             >> $OUTPATH/sc_spell_lists${PTR:+_ptr}.inc
./dbc_extract.py -p $INPUT -b $BUILD$PTR -t set_list               >> $OUTPATH/sc_spell_lists${PTR:+_ptr}.inc
./dbc_extract.py -p $INPUT --itemcache=$INPUT -b $BUILD$PTR -t item > $OUTPATH/sc_item_data${PTR:+_ptr}.inc
./dbc_extract.py -p $INPUT -b $BUILD$PTR -t random_property_points >> $OUTPATH/sc_item_data${PTR:+_ptr}.inc
./dbc_extract.py -p $INPUT -b $BUILD$PTR -t random_suffix          >> $OUTPATH/sc_item_data${PTR:+_ptr}.inc
./dbc_extract.py -p $INPUT -b $BUILD$PTR -t item_ench              >> $OUTPATH/sc_item_data${PTR:+_ptr}.inc
./dbc_extract.py -p $INPUT -b $BUILD$PTR -t item_armor             >> $OUTPATH/sc_item_data${PTR:+_ptr}.inc
./dbc_extract.py -p $INPUT -b $BUILD$PTR -t weapon_damage          >> $OUTPATH/sc_item_data${PTR:+_ptr}.inc
./dbc_extract.py -p $INPUT -b $BUILD$PTR -t gem_properties         >> $OUTPATH/sc_item_data${PTR:+_ptr}.inc
