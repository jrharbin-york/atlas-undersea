
###########################################################
#   Top level CMakeLists.txt file for the core-moos Project
#   pnewman@robots.ox.ac.uk
###########################################################

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

if(COMMAND cmake_policy)
   cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

set(PROJECTNAME MOOS)

set(PACKAGE_VERSION_MAJOR 10)
set(PACKAGE_VERSION_MINOR 0)
set(PACKAGE_VERSION_PATCH 3)

set(PACKAGE_VERSION "${PACKAGE_VERSION_MAJOR}.${PACKAGE_VERSION_MINOR}.${PACKAGE_VERSION_PATCH}")
message(STATUS "+++  This MOOS version ${PACKAGE_VERSION} +++")

# If user specifies the build type, use theirs, otherwise use release
if (NOT DEFINED CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "")
endif()


project(${PROJECTNAME})



if(NOT DEFINED CMAKE_RUNTIME_OUTPUT_DIRECTORY)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
endif()

if(NOT DEFINED CMAKE_LIBRARY_OUTPUT_DIRECTORY)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
endif()
if(NOT DEFINED CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
endif()


set(CMakeScripts_MODULE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(CMAKE_MODULE_PATH ${CMakeScripts_MODULE_DIR})

# Look at system to see what if any options are available because
# of build environment
include(SystemDefines)

# JRH: build for ActiveMQ and APR lib
#find_path(APR_INCLUDE_DIR
#  NAMES "apr_lib.h"
#  HINTS
#  "/usr/include/apr-1"
#  "/home/userfs/j/jrh512/source/apr-install/include/apr-1"
#  DOC "APR include files")
#
#find_library(ActiveMQ_LIBRARY
#  NAMES activemq-cpp
#  HINTS
#  /home/userfs/j/jrh512/source/activemq-cpp-install/lib
#  DOC "ActiveMQ library")
#
#find_path(ActiveMQ_INCLUDE_DIR
#  NAMES "ActiveMQCPP.h"
#  HINTS
#  "/home/userfs/j/jrh512/source/activemq-cpp-install/include/activemq-cpp-3.9.5/"
#  "/usr/local/include/activemq-cpp-3.9.5/"
#  PATH_SUFFIXES "activemq/library/"
#  DOC "ActiveMQ include files")
#
#find_library(ActiveMQ_LIBRARY
#  NAMES activemq-cpp
#  HINTS
#  /home/userfs/j/jrh512/source/activemq-cpp-install/lib
#  DOC "ActiveMQ library")

include_directories(${APR_INCLUDE_DIR} ${ActiveMQ_INCLUDE_DIR})
include_directories("/home/userfs/j/jrh512/source/activemq-cpp-install/include/activemq-cpp-3.9.5/")

set(CMAKE_CXX_STANDARD 11)

target_link_libraries(${ActiveMQ_LIBRARY})


# Turn on all compiler warnings
include(EnableAllWarnings)

# Bring in helper functions for dealing with CACHE INTERNAL variables
include(CacheInternalHelpers)

#figure out our git version
option(UPDATE_GIT_VERSION_INFO "update git version info in source tree" ON)
mark_as_advanced(UPDATE_GIT_VERSION_INFO)
if(UPDATE_GIT_VERSION_INFO)
	include(GitInfo)
endif()

# We want to create dynamic libraries
set(BUILD_SHARED_LIBS true)

# Enable Doxygen build with 'make doxygen'
option(ENABLE_DOXYGEN "Enable a 'make doc' target for Doxygen documentation")
if (ENABLE_DOXYGEN)
    include(UseDoxygen)
endif()

# Enable CDash testing 
option(CDASH_SUPPORT "Turn on testing targets that upload results to CDash" OFF)
if( CDASH_SUPPORT )
  include(SystemUtilityFunctions)
  moos_make_informative_build_name(CTEST_BUILD_NAME)
  # BUILDNAME is what is required for CTest
  set(BUILDNAME ${CTEST_BUILD_NAME})
  enable_testing()
  include( CTest )
endif()

# Option for code coverage
option(CODE_COVERAGE "Build code coverage results, requires GCC compiler (forces Debug build)" OFF)
if(CODE_COVERAGE)
  if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS_DEBUG
      "${CMAKE_CXX_FLAGS_DEBUG} -fprofile-arcs -ftest-coverage")
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)

    # Ensure that CDash targets are always enabled if coverage is enabled.
    if (NOT CDASH_SUPPORT)
      get_property(HELP_STRING CACHE CDASH_SUPPORT PROPERTY HELPSTRING)
      set(CDASH_SUPPORT ON CACHE BOOL "${HELP_STRING}" FORCE)
      message(STATUS "Enabling CDash targets as coverage has been enabled.")
    endif()
  endif(CMAKE_COMPILER_IS_GNUCXX)
endif()

# Option for explicitly linking against the GNU libstdc++ instead of libc++ on
# OSX Mavericks, required in order to use iMatlab as MATLAB still uses libstdc++
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  # On OSX, get the version number.
  find_program(SW_VERS_PROGRAM sw_vers)
  mark_as_advanced(SW_VERS_PROGRAM)
  execute_process(COMMAND ${SW_VERS_PROGRAM} "-productVersion"
    OUTPUT_VARIABLE OSX_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE)

  # Mavericks (10.9) and above use libc++ as the default.
  set(OSX_MAVERICKS_VERSION 10.9)

  if (OSX_VERSION VERSION_EQUAL OSX_MAVERICKS_VERSION OR
      OSX_VERSION VERSION_GREATER OSX_MAVERICKS_VERSION)
    option(USE_LIBSTDCPP
      "Use (legacy) GNU libstdc++ on OSX >= ${OSX_MAVERICKS_VERSION}, instead of default LLVM libc++ (required for iMatlab)" OFF)
    if (USE_LIBSTDCPP)
      message("Running OS X Mavericks (specific version: ${OSX_VERSION}) using "
        "legacy GNU libstdc++ instead of default LLVM libc++, disable "
        "USE_LIBSTDCPP option to revert to default LLVM libc++.")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libstdc++")
    else()
      # Remove any previous specification of libstdc++.
      message(STATUS "Using default LLVM libc++.")
      string(REPLACE "-stdlib=libstdc++" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    endif()
  endif()
endif()


###########  SETTING UP PREPROCESSOR ################
include(PlatformDefines)


#####################################################
# Toby Schneider's request for fPIC on amd64
IF("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64")
 SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC"  )
ENDIF("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64")


add_subdirectory(Core)


#########################
# Config Script Creation
#########################

# Add your library's name to PROJECT_EXPORT_LIBS if you wish to have it exported
# (This is a CACHE INTERNAL variable)
option(ENABLE_EXPORT "do you want CMake to record this build so other projects can find it " ON)
if(ENABLE_EXPORT)
   message(STATUS "location of this build will be exported, set ENABLE_EXPORT to OFF if you don't want this and delete ~/.cmake/projects/MOOS ")
endif()

if(1)
    if (PROJECT_EXPORT_LIBS)
        
        message(STATUS "PROJECT_EXPORT_LIBS = ${PROJECT_EXPORT_LIBS}")
        
        # Provide a cmake config script for the build directory
        export(TARGETS
            ${PROJECT_EXPORT_LIBS}
            FILE
            Use${PROJECTNAME}.cmake
        )
    
        set(CONFIG_FILE "${PROJECT_BINARY_DIR}/${PROJECTNAME}Config.cmake")
        configure_file( "cmake/ProjectConfig.cmake.in" ${CONFIG_FILE} @ONLY )
        
        # Append libary specific lines to the generated config script
        foreach(EXPORT_LIB ${PROJECT_EXPORT_LIBS})
            file(APPEND ${CONFIG_FILE} "\n# Export variables for ${EXPORT_LIB} Library\n")
            file(APPEND ${CONFIG_FILE} "set(${EXPORT_LIB}_LIBRARIES \"${${EXPORT_LIB}_LIBRARIES}\")\n")
            file(APPEND ${CONFIG_FILE} "set(${EXPORT_LIB}_INCLUDE_DIRS \"${${EXPORT_LIB}_INCLUDE_DIRS}\")\n")
            file(APPEND ${CONFIG_FILE} "set(${EXPORT_LIB}_DEPEND_LIBRARIES \"${${EXPORT_LIB}_DEPEND_LIBRARIES}\")\n")
            file(APPEND ${CONFIG_FILE} "set(${EXPORT_LIB}_DEPEND_INCLUDE_DIRS \"${${EXPORT_LIB}_DEPEND_INCLUDE_DIRS}\")\n")
        endforeach()
        
        foreach(DEFINE ${PROJECT_EXPORT_DEFINES})        
            file(APPEND ${CONFIG_FILE} "ADD_DEFINITIONS(-D${DEFINE})\n")        
            #file(APPEND ${CONFIG_FILE}  "message(STATUS \"adding -D${DEFINE} as required my MOOS\")\n" )
        endforeach()
        
        
    endif()

	if(ENABLE_EXPORT)
		# Stick build location in the central CMake user package registry, so that it may be easily found
		if ("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}" GREATER 2.7)
		  export(PACKAGE ${PROJECTNAME})
		endif()
	endif()

endif()

#here we save a version file
set(VERSION_FILE "${PROJECT_BINARY_DIR}/${PROJECTNAME}ConfigVersion.cmake")
configure_file( "cmake/ProjectConfigVersion.cmake.in" ${VERSION_FILE} @ONLY )




