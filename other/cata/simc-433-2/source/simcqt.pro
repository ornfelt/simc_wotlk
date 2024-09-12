TEMPLATE = app
TARGET = SimulationCraft
QT += core gui network webkit
#CONFIG += paperdoll
#CONFIG += openssl

exists( build.conf ) {
  include( build.conf )
}

QMAKE_CXXFLAGS_RELEASE += -DNDEBUG

win32 {
  LIBS += -lwininet
  RC_FILE += simcqt.rc

  # OpenSSL stuff:
  OPENSSL_INCLUDES = C:/OpenSSL-Win32/include
  OPENSSL_LIBS = C:/OpenSSL-Win32/lib
}

macx {
  QMAKE_INFO_PLIST = qt/Simulationcraft.plist
  ICON = qt/icon/Simcraft2.icns
  OBJECTIVE_SOURCES += qt/sc_mac_update.mm
  LIBS += -framework CoreFoundation -framework Sparkle -framework AppKit
}

COMPILER_CHECK_CXX = $$replace(QMAKE_CXX,'.*g\\+\\+'.*,'g++')

contains(COMPILER_CHECK_CXX,'g++') {
  QMAKE_CXXFLAGS += -ffast-math
}

INCLUDEPATH += engine
DEPENDPATH += engine

HEADERS += engine/simulationcraft.h
HEADERS += engine/data_enums.hh
HEADERS += engine/data_definitions.hh
HEADERS += engine/utf8.h
HEADERS += engine/utf8/core.h
HEADERS += engine/utf8/checked.h
HEADERS += engine/utf8/unchecked.h
HEADERS += qt/sc_autoupdate.h
HEADERS += qt/simulationcraftqt.h

SOURCES += engine/sc_action.cpp
SOURCES += engine/sc_armory.cpp
SOURCES += engine/sc_attack.cpp
SOURCES += engine/sc_battle_net.cpp
SOURCES += engine/sc_bcp_api.cpp
SOURCES += engine/sc_buff.cpp
SOURCES += engine/sc_chardev.cpp
SOURCES += engine/sc_chart.cpp
SOURCES += engine/sc_const_data.cpp
SOURCES += engine/sc_consumable.cpp
SOURCES += engine/sc_data.cpp
SOURCES += engine/sc_death_knight.cpp
SOURCES += engine/sc_dot.cpp
SOURCES += engine/sc_druid.cpp
SOURCES += engine/sc_enchant.cpp
SOURCES += engine/sc_event.cpp
SOURCES += engine/sc_expressions.cpp
SOURCES += engine/sc_gear_stats.cpp
SOURCES += engine/sc_heal.cpp
SOURCES += engine/sc_http.cpp
SOURCES += engine/sc_hunter.cpp
SOURCES += engine/sc_item.cpp
SOURCES += engine/sc_item_data.cpp
SOURCES += engine/sc_js.cpp
SOURCES += engine/sc_log.cpp
SOURCES += engine/sc_mage.cpp
SOURCES += engine/sc_mmo_champion.cpp
SOURCES += engine/sc_monk.cpp
SOURCES += engine/sc_option.cpp
SOURCES += engine/sc_paladin.cpp
SOURCES += engine/sc_pet.cpp
SOURCES += engine/sc_player.cpp
SOURCES += engine/sc_plot.cpp
SOURCES += engine/sc_priest.cpp
SOURCES += engine/sc_raid_event.cpp
SOURCES += engine/sc_rating.cpp
SOURCES += engine/sc_rawr.cpp
SOURCES += engine/sc_reforge_plot.cpp
SOURCES += engine/sc_report_html_player.cpp
SOURCES += engine/sc_report_html_sim.cpp
SOURCES += engine/sc_report_text.cpp
SOURCES += engine/sc_report_xml.cpp
SOURCES += engine/sc_report.cpp
SOURCES += engine/sc_rng.cpp
SOURCES += engine/sc_rogue.cpp
SOURCES += engine/sc_sample_data.cpp
SOURCES += engine/sc_scaling.cpp
SOURCES += engine/sc_sequence.cpp
SOURCES += engine/sc_set_bonus.cpp
SOURCES += engine/sc_shaman.cpp
SOURCES += engine/sc_sim.cpp
SOURCES += engine/sc_spell.cpp
SOURCES += engine/sc_spell_data.cpp
SOURCES += engine/sc_spell_info.cpp
SOURCES += engine/sc_stats.cpp
SOURCES += engine/sc_talent.cpp
SOURCES += engine/sc_target.cpp
SOURCES += engine/sc_thread.cpp
SOURCES += engine/sc_timespan.cpp
SOURCES += engine/sc_unique_gear.cpp
SOURCES += engine/sc_util.cpp
SOURCES += engine/sc_warlock.cpp
SOURCES += engine/sc_warrior.cpp
SOURCES += engine/sc_weapon.cpp
SOURCES += engine/sc_wowhead.cpp
SOURCES += engine/sc_xml.cpp
SOURCES += qt/main.cpp
SOURCES += qt/sc_window.cpp

CONFIG(paperdoll) {
  DEFINES += SC_PAPERDOLL
  HEADERS += qt/simcpaperdoll.h
  SOURCES += qt/simcpaperdoll.cc
}

CONFIG(openssl) {
  DEFINES += SC_USE_OPENSSL
  INCLUDEPATH += $$OPENSSL_INCLUDES
  LIBS += -L$$OPENSSL_LIBS -lssleay32
}
