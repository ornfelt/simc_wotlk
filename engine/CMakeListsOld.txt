cmake_minimum_required(VERSION 3.10)
project(simc CXX)

# Set compiler flags and options
set(CMAKE_CXX_STANDARD 11)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Woverloaded-virtual -Wno-unused-parameter -Wswitch-enum")

# Options for 32-bit or 64-bit
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -malign-double")
endif()

# Define source files
set(SOURCES
    simulationcraft.h
    sc_action.cpp
    sc_armory.cpp
    sc_attack.cpp
    sc_buff.cpp
    sc_chart.cpp
    sc_consumable.cpp
    sc_death_knight.cpp
    sc_druid.cpp
    sc_enchant.cpp
    sc_expressions.cpp
    sc_event.cpp
    sc_gear_stats.cpp
    sc_hunter.cpp
    #sc_http.cpp
    sc_item.cpp
    sc_js.cpp
    sc_log.cpp
    sc_mage.cpp
    sc_main.cpp
    sc_mmo_champion.cpp
    sc_option.cpp
    sc_paladin.cpp
    sc_pet.cpp
    sc_player.cpp
    sc_plot.cpp
    sc_priest.cpp
    sc_raid_event.cpp
    sc_rating.cpp
    sc_rawr.cpp
    sc_report.cpp
    sc_rng.cpp
    sc_rogue.cpp
    sc_shaman.cpp
    sc_scaling.cpp
    sc_sequence.cpp
    sc_set_bonus.cpp
    sc_sim.cpp
    sc_spell.cpp
    sc_stats.cpp
    sc_target.cpp
    sc_thread.cpp
    sc_unique_gear.cpp
    sc_util.cpp
    sc_warlock.cpp
    sc_warrior.cpp
    sc_weapon.cpp
    sc_wowhead.cpp
    sc_xml.cpp
)

# Define the executable and library
add_library(simc STATIC ${SOURCES})
set_target_properties(simc PROPERTIES OUTPUT_NAME "libsimc")
#target_link_libraries(simc pthread curl)

# Define the executable
add_executable(simc_exe ${SOURCES})
#target_link_libraries(simc_exe pthread curl)

# Installation
install(TARGETS simc simc_exe DESTINATION lib)
install(FILES ${SOURCES} DESTINATION include)

# Packaging
set(PACKAGE_VERSION "0")
add_custom_target(tarball
    COMMAND tar -cvf simc-r${PACKAGE_VERSION}.tar ${SOURCES} CMakeLists.txt
    COMMAND gzip simc-r${PACKAGE_VERSION}.tar
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Clean targets
#add_custom_target(mostlyclean
#    COMMAND rm -f ${CMAKE_BINARY_DIR}/libsimc.a ${CMAKE_BINARY_DIR}/simc_exe
#)

#add_custom_target(clean
#    COMMAND ${CMAKE_COMMAND} --build . --target mostlyclean
#    COMMAND rm -f ${CMAKE_BINARY_DIR}/libsimc.a ${CMAKE_BINARY_DIR}/simc_exe
#)
