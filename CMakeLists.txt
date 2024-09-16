cmake_minimum_required(VERSION 3.14)

# Project name
project(simc)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define configurations (Debug and Release)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)

# Define include directories (applies to both Debug and Release)
include_directories(
    #c:/simulationcraft
    #c:/simulationcraft/vs
    #c:/simulationcraft/sfmt
    #C:/data/code/simcraft/simulationcraft
    #C:/data/code/simcraft/simulationcraft/vs
    #C:/data/code/simcraft/simulationcraft/sfmt
    #./vs
)

# Source files
set(SOURCES
    engine/sc_action.cpp
    engine/sc_armory.cpp
    engine/sc_attack.cpp
    engine/sc_buff.cpp
    engine/sc_chart.cpp
    engine/sc_consumable.cpp
    engine/sc_death_knight.cpp
    engine/sc_druid.cpp
    engine/sc_enchant.cpp
    engine/sc_event.cpp
    engine/sc_expressions.cpp
    engine/sc_gear_stats.cpp
    engine/sc_http.cpp
    engine/sc_hunter.cpp
    engine/sc_item.cpp
    engine/sc_js.cpp
    engine/sc_log.cpp
    engine/sc_mage.cpp
    engine/sc_main.cpp
    engine/sc_mmo_champion.cpp
    engine/sc_option.cpp
    engine/sc_paladin.cpp
    engine/sc_pet.cpp
    engine/sc_player.cpp
    engine/sc_plot.cpp
    engine/sc_priest.cpp
    engine/sc_raid_event.cpp
    engine/sc_rating.cpp
    engine/sc_rawr.cpp
    engine/sc_report.cpp
    engine/sc_rng.cpp
    engine/sc_rogue.cpp
    engine/sc_scaling.cpp
    engine/sc_sequence.cpp
    engine/sc_set_bonus.cpp
    engine/sc_shaman.cpp
    engine/sc_sim.cpp
    engine/sc_spell.cpp
    engine/sc_stats.cpp
    engine/sc_target.cpp
    engine/sc_thread.cpp
    engine/sc_unique_gear.cpp
    engine/sc_util.cpp
    engine/sc_warlock.cpp
    engine/sc_warrior.cpp
    engine/sc_weapon.cpp
    engine/sc_wowhead.cpp
    engine/sc_xml.cpp
)

# Header files (for precompiled headers, etc.)
set(HEADERS
    engine/simulationcraft.h
)

# Precompiled headers (optional, can be configured in CMake 3.16+)
# target_precompile_headers(${PROJECT_NAME} PUBLIC engine/simulationcraft.h)

# Define the executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Link options and libraries
target_link_libraries(${PROJECT_NAME} wininet)

# Compiler options per configuration
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Debug>:
        /DWIN32 /D_DEBUG /D_CONSOLE /D_WINDOWS
        /MTd
        /Zi /Od /RTC1 /MP
    >
    $<$<CONFIG:Release>:
        /DWIN32 /DNDEBUG /D_CONSOLE /D_WINDOWS
        /MT
        /O2 /Ob2 /Oi /Ot /Oy /GL /DNDEBUG
        /MP /GS /Gy
    >
)

# Linker options per configuration
target_link_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Debug>:
        /SUBSYSTEM:CONSOLE /MACHINE:X86
    >
    $<$<CONFIG:Release>:
        /SUBSYSTEM:CONSOLE /MACHINE:X86 /OPT:REF /OPT:ICF /LTCG
    >
)

# Set the output directory
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/Debug"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/Release"
)
