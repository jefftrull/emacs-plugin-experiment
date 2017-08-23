cmake_minimum_required( VERSION 3.5 )

set( CMAKE_CXX_STANDARD 14 )
set( CMAKE_CXX_FLAGS "-Wall -Wextra -Werror" )
set( CMAKE_CXX_FLAGS_DEBUG "-g -Og -fno-omit-frame-pointer" )

if( CMAKE_BUILD_TYPE STREQUAL "" )
  set( CMAKE_BUILD_TYPE Release CACHE STRING "Build type (Release, Debug, RelWithDebInfo, MinSizeRel)" FORCE )
endif()

if( NOT EMACS_ROOT )
  message( FATAL_ERROR "You must specify the emacs build location with -DEMACS_ROOT=..." )
endif()

add_library( dymod SHARED dymod.cpp )
set_target_properties( dymod PROPERTIES
  INCLUDE_DIRECTORIES "${EMACS_ROOT}/src"    # to find emacs-module.h
)
