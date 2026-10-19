# Based on the CMakeLists.txt of the Geant4 advanced example IAEAphsp
# (M.A. Cortes-Giraldo); modified by S. Lopez Martinez, University College London, 2026.
#----------------------------------------------------------------------------
# Setup the project
cmake_minimum_required(VERSION 3.16...3.27)
project(phantoms_sim)

#----------------------------------------------------------------------------
# Find Geant4 package, activating all available UI and Vis drivers by default
# You can set WITH_GEANT4_UIVIS to OFF via the command line or ccmake/cmake-gui
# to build a batch mode only executable
#
option(WITH_GEANT4_UIVIS "Build example with Geant4 UI and Vis drivers" ON)
if(WITH_GEANT4_UIVIS)
  find_package(Geant4 REQUIRED ui_all vis_all)
else()
  find_package(Geant4 REQUIRED)
endif()

#----------------------------------------------------------------------------
# Setup Geant4 include directories and compile definitions
#
include(${Geant4_USE_FILE})

#----------------------------------------------------------------------------
# Locate sources and headers for this project
#
include_directories(${PROJECT_SOURCE_DIR}/include
                    ${PROJECT_SOURCE_DIR}/iaea_phsp
                    ${Geant4_INCLUDE_DIR})
file(GLOB sources ${PROJECT_SOURCE_DIR}/source/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.hh)

#----------------------------------------------------------------------------
# Build the iaea_phsp support library
add_subdirectory(iaea_phsp)

#----------------------------------------------------------------------------
# Add the executable, and link it to the Geant4 libraries
#
add_executable(phantoms_sim phantoms_sim.cc ${sources} ${headers})
target_link_libraries(phantoms_sim iaea_phsp ${Geant4_LIBRARIES})

#----------------------------------------------------------------------------
# Add the macros from the macros directory
#
file(GLOB macros CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/macros/*.mac)
file(COPY ${macros} DESTINATION ${PROJECT_BINARY_DIR})
# Phase-space files are NOT copied into the build (they are tens of GB). Build in
# <package>/build (cmake ..); the macros read and write them in <package>/phsp
# as ../phsp/... (create that directory first).
# macros/final_runs/ is not copied: run_final_productions.sh executes those
# macros from the source tree.

#----------------------------------------------------------------------------
# Install the executable to 'bin' directory under CMAKE_INSTALL_PREFIX
#
install(TARGETS phantoms_sim DESTINATION bin)
