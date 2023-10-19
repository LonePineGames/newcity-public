
# CMake entry point
cmake_minimum_required (VERSION 3.0)
project (NewCity)
if (POLICY CMP0072)
  cmake_policy(SET CMP0072 NEW)
endif()

#set(CMAKE_DEBUG_TARGET_PROPERTIES
#  INCLUDE_DIRECTORIES
#  COMPILE_DEFINITIONS
#  POSITION_INDEPENDENT_CODE
#  CONTAINER_SIZE_REQUIRED
#  LIB_VERSION
#)

set(OPENAL_LIBRARY lib)
set(OPENAL_INCLUDE_DIR external/openal-soft/include)

find_package(OpenGL REQUIRED)
find_package(OpenAL REQUIRED)
find_package(Threads REQUIRED)

if( CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR )
    message( FATAL_ERROR "Please don't use source dir as build dir")
endif()
if( CMAKE_SOURCE_DIR MATCHES " " )
	message( "Your Source Directory contains spaces. If you experience problems when compiling, this can be the cause." )
endif()
if( CMAKE_BINARY_DIR MATCHES " " )
	message( "Your Build Directory contains spaces. If you experience problems when compiling, this can be the cause." )
endif()

if (LP_IS_MINGW)
  find_library (PSAPI Psapi)
  find_package (dlfcn-win32 REQUIRED)
endif()

# Compile external dependencies
add_subdirectory (external)

# On Visual 2005 and above, this module can set the debug working directory
#cmake_policy(SET CMP0026 OLD)
#list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/external/rpavlik-cmake-modules-fe2273")
#include(CreateLaunchers)
#include(MSVCMultipleProcessCompile) # /MP

# Include freetype.lib cmake
include(${CMAKE_SOURCE_DIR}/external/freetype/CMakeLists.txt)

set(Boost_USE_STATIC_LIBS ON)
set(Boost_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/external/boost-install/include)
set(BOOST_LIBRARYDIR ${CMAKE_SOURCE_DIR}/external/boost-install/lib)
set(BOOST_ROOT ${CMAKE_SOURCE_DIR}/external/boost-install)
include_directories(${Boost_INCLUDE_DIR})
link_directories(${CMAKE_SOURCE_DIR}/lib ${Boost_LIBRARYDIR})
#find_package(Boost COMPONENTS unit_test_framework REQUIRED)

# Import Steam based on the compiler toolchain
# Commented out if statements to link both, going to handle with preproc in code
# if (LP_IS_MINGW)
  # Import Steam for Linux
  add_library(steamapi_linux SHARED IMPORTED)
  set_target_properties(steamapi_linux PROPERTIES
  IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/external/steam/redistributable_bin/linux64/libsteam_api.so"
  IMPORTED_IMPLIB "${CMAKE_SOURCE_DIR}/external/steam/lib/linux64/libsteam_api.so"
  INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/external/steam/include")
# elseif (WIN32)
  # Import Steam for Win64
  add_library(steamapi SHARED IMPORTED)
  set_target_properties(steamapi PROPERTIES
  IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/external/steam/redistributable_bin/win64/steam_api64.dll"
  IMPORTED_IMPLIB "${CMAKE_SOURCE_DIR}/external/steam/lib/win64/steam_api64.lib"
  INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/external/steam/include")
# else()
#  message( "Invalid or unknown compiler toolchain for linking Steamworks" )
# endif()

# Link the appropriate Steam library for the target
if( UNIX )
  set(STEAM 
  steamapi_linux)
elseif( WIN32 )
  set(STEAM 
  steamapi)
else()
  message( "Invalid or unknown target for linking Steamworks" )
endif()

# Include StackWalker
if(WIN32)
  include("${CMAKE_SOURCE_DIR}/external/StackWalker/CMakeLists.txt")
endif()

include_directories(SYSTEM
  external/boost/boost_1_74_0/
  external/curl/include/
  external/glfw-3.3/include/GLFW/
  external/glm-0.9.7.1/
  external/glew-1.13.0/include/
  external/openal-soft/include/
  external/spdlog-1.5.0/include/
  external/gcc/libbacktrace/
  external/lua-5.3.5/src/
  external/freetype/include/
  external/steam/include
  external/StackWalker/include
  .
)

set(ALL_LIBS
  ${Boost_LIBRARIES}
  ${OPENGL_LIBRARY}
  glfw
  GLEW_1130
  openal
  freetype
  ${STEAM}
)

if ( WIN32 )
  set(ALL_LIBS ${ALL_LIBS} stackwalker)
endif()

if ( LP_IS_MINGW )
  set(ALL_LIBS ${ALL_LIBS} dlfcn-win32::dl backtrace)
endif()

if ( UNIX )
  set(ALL_LIBS ${ALL_LIBS} stdc++fs)
  #set(ALL_LIBS ${ALL_LIBS} backtrace)
endif()

add_definitions(
  #  -DFOO
  -DTW_STATIC
  -DTW_NO_LIB_PRAGMA
  -DTW_NO_DIRECT3D
  -DGLEW_STATIC
  -D_CRT_SECURE_NO_WARNINGS
)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif(NOT CMAKE_BUILD_TYPE)

if(WIN32)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /we4716 /O2 /Zi /MP /Qpar")
endif()

if(NOT WIN32)
  SET(CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} -g -std=c++14")
	SET(CMAKE_CXX_FLAGS
  "${CMAKE_CXX_FLAGS} -Werror=return-type") #-Wall
SET(CMAKE_CXX_FLAGS
  "${CMAKE_CXX_FLAGS} -Wshadow")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSPDLOG_NO_EXCEPTIONS=ON")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBOOST_EXCEPTION_DISABLE")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBOOST_NO_EXCEPTIONS")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DINCLUDE_STEAM")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -D_GLIBCXX_DEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DLP_DEBUG")
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -ffast-math")
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer")

if (UNIX)
  set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -static")
endif()

if(CMAKE_BUILD_TYPE MATCHES Release)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -fno-rtti -fno-exceptions")
endif()

file(GLOB LUA_SOURCE external/lua-5.3.5/src/*.c)

# Main Source
SET(ALL_FILES
  ${LUA_SOURCE}

  src/amenity.cpp
  src/blueprint.cpp
  src/board.cpp
  src/box.cpp
  src/building/building.cpp
  src/building/buildingTexture.cpp
  src/building/buildingCallbacks.cpp
  src/building/deco.cpp
  src/building/design.cpp
  src/building/designOrganizer.cpp
  src/building/designPackage.cpp
  src/building/statue.cpp
  src/building/renderBuilding.cpp
  src/business.cpp
  src/city.cpp
  src/collisionTable.cpp
  src/color.cpp
  src/compass.cpp
  src/configuration.cpp
  src/economy.cpp
  src/newcity.rc

  src/console/conCallbacks.cpp
  src/console/conCmd.cpp
  src/console/conDisplay.cpp
  src/console/conInput.cpp
  src/console/conMod.cpp

  src/draw/buffer.cpp
  src/draw/camera.cpp
  src/draw/culler.cpp
  src/draw/drawCallbacks.cpp
  src/draw/entity.cpp
  src/draw/framebuffer.cpp
  src/draw/image.cpp
  src/draw/mesh.cpp
  src/draw/shader.cpp
  src/draw/texture.cpp

  src/error.cpp

  src/game/achievement.cpp
  src/game/constants.cpp
  src/game/feature.cpp
  src/game/game.cpp
  src/game/task.cpp
  src/game/update.cpp
  src/game/version.cpp

  src/graph.cpp
  src/graph/config.cpp
  src/graph/graphParent.cpp
  src/graph/stop.cpp
  src/graph/transit.cpp
  src/graph/transitRouter.cpp

  src/heatmap.cpp
  src/icons.cpp

  src/import/obj.cpp
  src/import/mesh-import.cpp

  src/input.cpp
  src/intersection.cpp
  src/item.cpp
  src/label.cpp
  src/land.cpp
  src/lane.cpp
  src/line.cpp
  src/log.cpp
  src/lot.cpp
  src/lz4.c
  src/main.cpp
  src/md4c/md4c.c
  src/money.cpp
  src/name.cpp

  src/newspaper/newspaper.cpp
  src/newspaper/newspaperCallbacks.cpp
  src/newspaper/article.cpp

  src/option.cpp

  src/parts/aboutPanel.cpp
  src/parts/amenityInfo.cpp
  src/parts/article.cpp
  src/parts/blank.cpp
  src/parts/block.cpp
  src/parts/blueprintPanel.cpp
  src/parts/blueprintsList.cpp
  src/parts/budgetPanel.cpp
  src/parts/changelogPanel.cpp
  src/parts/citipediaPanel.cpp
  src/parts/console.cpp
  src/parts/colorPicker.cpp
  src/parts/button.cpp
  src/parts/chart.cpp
  src/parts/designConfigPanel.cpp
  src/parts/economyPanel.cpp
  src/parts/error.cpp
  src/parts/hr.cpp
  src/parts/icon.cpp
  src/parts/image.cpp
  src/parts/label.cpp
  src/parts/leftPanel.cpp
  src/parts/loader.cpp
  src/parts/loadPreviewPanel.cpp

  src/parts/mainMenu.cpp
  src/parts/menuBar.cpp
  src/parts/messageBoard.cpp
  src/parts/newGamePanel.cpp
  src/parts/newspaperPanel.cpp
  src/parts/optInPopup.cpp
  src/parts/optionsPanel.cpp
  src/parts/root.cpp
  src/parts/panel.cpp
  src/parts/part.cpp
  src/parts/renderParts.cpp
  src/parts/scrollbox.cpp
  src/parts/selectionPanel.cpp
  src/parts/slider.cpp
  src/parts/span.cpp
  src/parts/statusBar.cpp
  src/parts/steamWorkshop.cpp
  src/parts/textBox.cpp
  src/parts/textureSelect.cpp
  src/parts/toolbar.cpp
  src/parts/tooltip.cpp
  src/parts/transitPanel.cpp
  src/parts/transitDesignerPanel.cpp
  src/parts/tutorialPanel.cpp
  src/parts/tutorialPopup.cpp

  src/person.cpp
  src/pillar.cpp
  src/plan.cpp

  src/platform/binaryfilebuilder.cpp
  src/platform/binaryfilereader.cpp
  src/platform/event.cpp
  src/platform/fcaseopen.cpp
  src/platform/file.cpp
  src/platform/lookup.cpp
  src/platform/lua.cpp
  src/platform/mod.cpp

  src/pool.cpp

  src/renderGraph.cpp
  src/renderLot.cpp
  src/renderPillar.cpp
  src/renderPlan.cpp
  src/renderSelection.cpp
  src/renderUtils.cpp

  src/renum.cpp

  src/route/broker.cpp
  src/route/heap.cpp
  src/route/location.cpp
  src/route/route.cpp
  src/route/router.cpp
  src/route/routeRequest.cpp

  src/simplex.cpp
  src/selection.cpp
  src/serialize.cpp
  src/sound.cpp
  src/spline.cpp

  src/steam/steamwrapper.cpp
  src/steam/steamws.cpp
  src/steam/steamws_core.cpp

  src/string.cpp
  src/string_proxy.cpp
  src/tanker.cpp
  src/terrain/tree.cpp
  src/thread.cpp
  src/time.cpp
  src/tutorial.cpp

  src/tools/blueprint.cpp
  src/tools/elevation.cpp
  src/tools/road.cpp
  src/tools/plansPanel.cpp

  src/util.cpp

  src/vehicle/interpolator.cpp
  src/vehicle/laneLoc.cpp
  src/vehicle/model.cpp
  src/vehicle/pedestrian.cpp
  src/vehicle/physics.cpp
  src/vehicle/renderPedestrian.cpp
  src/vehicle/renderVehicle.cpp
  src/vehicle/transitPhysics.cpp
  src/vehicle/travelGroup.cpp
  src/vehicle/update.cpp
  src/vehicle/vehicle.cpp
  src/vehicle/vehicleCallbacks.cpp
  src/vehicle/wanderer.cpp

  src/vorbis_proxy.cpp
  src/weather.cpp
  src/zone.cpp
)

add_executable(newcity ${ALL_FILES})

enable_testing()
add_executable(game_test 
  src/game/test.cpp
)

target_link_libraries(newcity
	${ALL_LIBS}
)

target_link_libraries(game_test
)

add_test(game_test game_test)

if (LP_IS_MINGW)
  target_link_libraries(newcity -lpsapi)
endif()

if(THREADS_HAVE_PTHREAD_ARG)
  target_compile_options(newcity PUBLIC "-pthread")
endif()
if(CMAKE_THREAD_LIBS_INIT)
  target_link_libraries(newcity "${CMAKE_THREAD_LIBS_INIT}")
endif()

# Xcode and Visual working directories
#set_target_properties(newcity PROPERTIES XCODE_ATTRIBUTE_CONFIGURATION_BUILD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/")
#create_target_launcher(newcity WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src/")

if(WIN32)
  if(CMAKE_BUILD_TYPE MATCHES Release OR CMAKE_BUILD_TYPE MATCHES RelWithDebInfo)
    set_target_properties(newcity PROPERTIES
      LINK_FLAGS "/SUBSYSTEM:windows /DEBUG /PDB:newcity.pdb /PDBSTRIPPED:newcity_stripped.pdb /PDBALTPATH:%_PDB% /STACK:8000000")
  else()
    set_target_properties(newcity PROPERTIES
      LINK_FLAGS "/SUBSYSTEM:CONSOLE /DEBUG /PDB:newcity.pdb /PDBSTRIPPED:newcity_stripped.pdb /PDBALTPATH:%_PDB% /STACK:8000000")
  endif()

  #set_target_properties(newcity PROPERTIES
  #LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")
  #set_target_properties(newcity PROPERTIES
  #COMPILE_DEFINITIONS_DEBUG "_CONSOLE")
  #set_target_properties(newcity PROPERTIES
  #LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:windows")
  #set_target_properties(newcity PROPERTIES
  #COMPILE_DEFINITIONS_RELWITHDEBINFO "_CONSOLE")
  #set_target_properties(newcity PROPERTIES
  #LINK_FLAGS_RELEASE "/SUBSYSTEM:windows")
  #set_target_properties(newcity PROPERTIES
  #LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:windows")
endif()

if (NOT ${CMAKE_GENERATOR} MATCHES "Xcode" )

# Copy steam dll to build dir based on compiler toolchain
# Commented out if statements so we copy both files every time
# if (LP_IS_MINGW)
  add_custom_command(
    TARGET newcity PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_SOURCE_DIR}/external/steam/redistributable_bin/linux64/libsteam_api.so
            ${CMAKE_CURRENT_BINARY_DIR}/libsteam_api.so)
# elseif (WIN32)
  add_custom_command(
    TARGET newcity PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_SOURCE_DIR}/external/steam/redistributable_bin/win64/steam_api64.dll
            ${CMAKE_CURRENT_BINARY_DIR}/steam_api64.dll)
# endif()

# Copy other redistributables
#add_custom_command(
#  TARGET newcity PRE_BUILD
#  COMMAND ${CMAKE_COMMAND} -E copy
#          ${CMAKE_SOURCE_DIR}/lib/OpenAL32.dll
#          ${CMAKE_CURRENT_BINARY_DIR}/OpenAL32.dll
#)

add_custom_command(
   TARGET newcity POST_BUILD
   COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/newcity${CMAKE_EXECUTABLE_SUFFIX}" "${CMAKE_CURRENT_SOURCE_DIR}/src/"
)
add_custom_command(
   TARGET game_test POST_BUILD
   COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/game_test${CMAKE_EXECUTABLE_SUFFIX}" "${CMAKE_CURRENT_SOURCE_DIR}/src/"
)

if(WIN32)
  # Include PDB for stacktraces
  add_custom_command(
    TARGET newcity POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/newcity.pdb" "${CMAKE_CURRENT_SOURCE_DIR}/src/"
  )
endif()

elseif (${CMAKE_GENERATOR} MATCHES "Xcode" )

endif (NOT ${CMAKE_GENERATOR} MATCHES "Xcode" )

message("CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}")
message("CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
message("CMAKE_LINKER_FLAGS: ${CMAKE_CXX_LINKER_FLAGS}")
message("CMAKE_EXE_LINKER_FLAGS: ${CMAKE_EXE_LINKER_FLAGS}")
