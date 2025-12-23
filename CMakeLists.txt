#
# osgThreeJSX CMake build file
#
SET(OSGTHREEJSX_MAJOR_VERSION 1)
SET(OSGTHREEJSX_MINOR_VERSION 0)
SET(OSGTHREEJSX_PATCH_VERSION 0)
SET(OSGTHREEJSX_SOVERSION 1)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.0 FATAL_ERROR)

if(COMMAND cmake_policy)
    # Works around warnings libraries linked against that don't
    # have absolute paths (e.g. -lpthreads)
    cmake_policy(SET CMP0003 NEW)

    # Works around warnings about escaped quotes in ADD_DEFINITIONS
    # statements.
    cmake_policy(SET CMP0005 NEW)

    # tell CMake to prefer CMake's own CMake modules when available
    # only available from cmake-2.8.4
    if("${CMAKE_VERSION}" VERSION_GREATER 2.8.3)
        cmake_policy(SET CMP0017 NEW)
    endif()

endif()

IF(APPLE)
    # Get OSX version in MAJOR.MINOR format
    EXECUTE_PROCESS(COMMAND sw_vers -productVersion
        OUTPUT_VARIABLE OSGTHREEJSX_OSX_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    STRING(REGEX REPLACE "^([0-9]+\\.[0-9]+).*$" "\\1"
        OSGTHREEJSX_OSX_VERSION "${OSGTHREEJSX_OSX_VERSION}")
    MESSAGE(STATUS "OSGTHREEJSX_OSX_VERSION=${OSGTHREEJSX_OSX_VERSION}")
ENDIF()

PROJECT(osgThreeJSX)

SET(ACTUAL_3RDPARTY_DIR CACHE STRING "3rdparty directory")
SET(OSG_DIR CACHE STRING "OpenSceneGraph directory")

IF(ANDROID)
    SET(OSGTHREEJSX_WINDOWING_SYSTEM "None" CACHE STRING "Windowing system type for graphics window creation; options: None.")
ELSEIF(WIN32 OR MINGW)
    SET(OSGTHREEJSX_WINDOWING_SYSTEM "Win32" CACHE STRING "Windowing system type for graphics window creation; options: Win32 or None.")
ELSEIF(APPLE)
    # custom option to flag an iOS build
    OPTION(OSGTHREEJSX_BUILD_PLATFORM_IPHONE "Enable IPhoneSDK Device support" OFF)

    IF(OSGTHREEJSX_BUILD_PLATFORM_IPHONE)

        # set sdk and min versions
        SET (IPHONE_SDKVER "10.2" CACHE STRING "IOS SDK-Version")
        SET (IPHONE_VERSION_MIN "7.0" CACHE STRING "IOS minimum os version, use 7.0 or greater to get 64bit support")

        # get full path to sdk from requested versions
        SET (IPHONE_DEVROOT "/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer")
        SET (IPHONE_SDKROOT "${IPHONE_DEVROOT}/SDKs/iPhoneOS${IPHONE_SDKVER}.sdk")

        # optionally enable bitcode for the build
        SET (IPHONE_ENABLE_BITCODE "NO" CACHE STRING "IOS Enable Bitcode")

        # seamless toggle between device and simulator
        SET(CMAKE_XCODE_EFFECTIVE_PLATFORMS "-iphoneos;-iphonesimulator" CACHE STRING "Xcode effective platforms for iOS")

        # set deployment target to min version
        SET(CMAKE_XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET "${IPHONE_VERSION_MIN}" CACHE STRING "Xcode deployment target for iOS")

        # Set standard architectures
        SET(CMAKE_OSX_ARCHITECTURES "$(ARCHS_STANDARD)" CACHE STRING "Build architectures for iOS")

    ELSE()
        # OSX >= 10.5 uses Cocoa windowing system, otherwise Carbon
        IF(OSGTHREEJSX_OSX_VERSION VERSION_LESS 10.5)
            SET(OSG_WINDOWING_SYSTEM "Carbon" CACHE STRING "Windowing system type for graphics window creation; options: Carbon, Cocoa, X11 or None.")
			SET(OSG_WINDOWING_SYSTEM_CARBON ON INTERNAL "use Carbon (apple; 32 bit only)")
        ELSE()
            SET(OSG_WINDOWING_SYSTEM "Cocoa" CACHE STRING "Windowing system type for graphics window creation; options: Carbon, Cocoa, X11 or None.")
        ENDIF()

        # Set macOS architectures for Universal Binaries.
        IF(OSGTHREEJSX_OSX_VERSION VERSION_GREATER_EQUAL 10.14)
            # x86_64 only
            SET(CMAKE_OSX_ARCHITECTURES "x86_64" CACHE STRING "Build architectures for OSX")
            SET(CMAKE_OSX_DEPLOYMENT_TARGET "10.9" CACHE STRING "Target OSX version")
            # libstdc++ is no longer supported
            SET(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++11")
            SET(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
        ELSEIF(OSGTHREEJSX_OSX_VERSION VERSION_GREATER_EQUAL 10.8)
            # i386 is no longer supported
            SET(CMAKE_OSX_ARCHITECTURES "x86_64" CACHE STRING "Build architectures for OSX")
            SET(CMAKE_OSX_DEPLOYMENT_TARGET "10.8" CACHE STRING "Target OSX version")
        ELSEIF(OSGTHREEJSX_OSX_VERSION VERSION_EQUAL 10.7)
            # ppc is no longer supported
            SET(CMAKE_OSX_ARCHITECTURES "i386;x86_64" CACHE STRING "Build architectures for OSX")
        ELSEIF(OSGTHREEJSX_OSX_VERSION VERSION_GREATER 10.4)
            # 64-bit compiles are not supported with Carbon.
            SET(CMAKE_OSX_ARCHITECTURES "ppc;i386" CACHE STRING "Build architectures for OSX")
            SET(CMAKE_OSX_DEPLOYMENT_TARGET "10.5" CACHE STRING "Target OSX version")
        ELSEIF(OSGTHREEJSX_OSX_VERSION VERSION_EQUAL 10.4)
            # 64-bit compiles are not supported with Carbon.
            SET(CMAKE_OSX_ARCHITECTURES "ppc;i386" CACHE STRING "Build architectures for OSX")
        ELSE()
            # No Universal Binary support and SDK detection is too unreliable.
            # Warn user and continue at their own peril.
            MESSAGE(WARNING "OSX 10.3 and earlier not supported.")
        ENDIF()
    ENDIF()
ELSE()
    SET(OSG_WINDOWING_SYSTEM "X11" CACHE STRING "Windowing system type for graphics window creation; options: X11 or None.")
ENDIF()

set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH)
SET(OSGTHREEJSX_VERSION ${OSGTHREEJSX_MAJOR_VERSION}.${OSGTHREEJSX_MINOR_VERSION}.${OSGTHREEJSX_PATCH_VERSION})

# We have some custom .cmake scripts not in the official distribution.
# Maybe this can be used override existing behavior if needed?
SET(CMAKE_MODULE_PATH "${osgThreeJSX_SOURCE_DIR}/CMakeModules;${CMAKE_MODULE_PATH}")
################################################################################
# 3rd Party Dependency Stuff
IF(WIN32 AND NOT ANDROID)
    INCLUDE(Find3rdPartyDependencies)    
    FIND_PACKAGE(draco)    
ENDIF()

FIND_PACKAGE(OSG)

SET(OSG_PLUGIN_PREFIX "")

SET(OSG_MAJOR_VERSION 3)
SET(OSG_MINOR_VERSION 1)
SET(OSG_PATCH_VERSION 1)
IF(OSG_INCLUDE_DIR)
    FILE(READ "${OSG_INCLUDE_DIR}/OpenThreads/Version" OT_FILE_CONTENTS)
    FILE(READ "${OSG_INCLUDE_DIR}/osg/Version" OSG_FILE_CONTENTS)
    STRING(FIND "${OT_FILE_CONTENTS}" "OPENTHREADS_SOVERSION" OT_VERSION_INDEX)
    STRING(FIND "${OSG_FILE_CONTENTS}" "OPENSCENEGRAPH_SOVERSION" OSG_VERSION_INDEX)
    STRING(FIND "${OSG_FILE_CONTENTS}" "OPENSCENEGRAPH_MAJOR_VERSION" OSGPLUGIN_INDEX0)
    STRING(FIND "${OSG_FILE_CONTENTS}" "OPENSCENEGRAPH_MINOR_VERSION" OSGPLUGIN_INDEX1)
    STRING(FIND "${OSG_FILE_CONTENTS}" "OPENSCENEGRAPH_PATCH_VERSION" OSGPLUGIN_INDEX2)

    MATH(EXPR OT_VERSION_INDEX "${OT_VERSION_INDEX} + 22")
    MATH(EXPR OSG_VERSION_INDEX "${OSG_VERSION_INDEX} + 25")
    MATH(EXPR OSGPLUGIN_INDEX0 "${OSGPLUGIN_INDEX0} + 29")
    MATH(EXPR OSGPLUGIN_INDEX1 "${OSGPLUGIN_INDEX1} + 29")
    MATH(EXPR OSGPLUGIN_INDEX2 "${OSGPLUGIN_INDEX2} + 29")
    STRING(SUBSTRING "${OT_FILE_CONTENTS}" "${OT_VERSION_INDEX}" "4" OT_VERSION)
    STRING(SUBSTRING "${OSG_FILE_CONTENTS}" "${OSG_VERSION_INDEX}" "12" OSG_VERSION)
    STRING(SUBSTRING "${OSG_FILE_CONTENTS}" "${OSGPLUGIN_INDEX0}" "5" OSG_MAJOR_VERSION)
    STRING(SUBSTRING "${OSG_FILE_CONTENTS}" "${OSGPLUGIN_INDEX1}" "5" OSG_MINOR_VERSION)
    STRING(SUBSTRING "${OSG_FILE_CONTENTS}" "${OSGPLUGIN_INDEX2}" "5" OSG_PATCH_VERSION)

    STRING(STRIP ${OT_VERSION} OT_VERSION)
    STRING(STRIP ${OSG_VERSION} OSG_VERSION)
    STRING(STRIP ${OSG_MAJOR_VERSION} OSG_MAJOR_VERSION)
    STRING(STRIP ${OSG_MINOR_VERSION} OSG_MINOR_VERSION)
    STRING(STRIP ${OSG_PATCH_VERSION} OSG_PATCH_VERSION)

    SET(INSTALL_PLUGINDIR "bin/osgPlugins-${OSG_MAJOR_VERSION}.${OSG_MINOR_VERSION}.${OSG_PATCH_VERSION}")
ENDIF(OSG_INCLUDE_DIR)

SET(OSG_PLUGINS osgPlugins-${OSG_MAJOR_VERSION}.${OSG_MINOR_VERSION}.${OSG_PATCH_VERSION})

# Change the default build type to Release
IF(NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)

IF(NOT ANDROID)
IF(APPLE)

    IF(OSG_BUILD_PLATFORM_IPHONE)

        # set the sdk path as our sysroot, if we set this before the project is defined cmake fails to build its test program
        SET(CMAKE_OSX_SYSROOT "${IPHONE_SDKROOT}" CACHE STRING "System root for iOS" FORCE)

        # Set the path to OpenGL library
        SET(OPENGL_gl_LIBRARY "${IPHONE_SDKROOT}/System/Library/Frameworks/OpenGLES.framework")

    ELSE ()
        FIND_LIBRARY(COCOA_LIBRARY Cocoa)

        # Apple OS X: Find OpenGL and AGL based on OSG_WINDOWING_SYSTEM
        # This is the accepted way of finding X11/OpenGL on OSX, as
        # documented in CMake's FindOpenGL module.
        # Note that without this check, libosg would use Cocoa/OpenGL but
        # libosgViewer would use X11/OpenGL, which causes compatibility
        # issues for applications using OSG.
        UNSET(OPENGL_gl_LIBRARY CACHE)
        UNSET(OPENGL_INCLUDE_DIR CACHE)
        IF(OSG_WINDOWING_SYSTEM STREQUAL "X11")
          FIND_PACKAGE(X11)
          IF(NOT X11_FOUND)
            MESSAGE(FATAL_ERROR "OSG_WINDOWING_SYSTEM is X11, but no X11 installation was found. Please make sure X11 is properly installed.")
          ENDIF()

          # Use X11 version of OpenGL as seed for CMake FindOpenGL
          GET_FILENAME_COMPONENT(X11LIBDIR ${X11_X11_LIB} DIRECTORY)
          FIND_LIBRARY(OPENGL_gl_LIBRARY GL PATHS ${X11LIBDIR} DOC "OpenGL lib for OSX" NO_DEFAULT_PATH)
          SET(OPENGL_INCLUDE_DIR ${X11_INCLUDE_DIR} CACHE PATH "Include for OpenGL on OSX" FORCE)

        ELSEIF(OSG_WINDOWING_SYSTEM STREQUAL "Carbon")
          # AGL needed for Carbon windowing systems
          FIND_LIBRARY(CARBON_LIBRARY Carbon)
          FIND_LIBRARY(AGL_LIBRARY AGL)
        ENDIF()

        FIND_PACKAGE(OpenGL)
        FIND_PACKAGE(EGL)
    ENDIF ()

    OPTION(OSG_COMPILE_FRAMEWORKS "compile frameworks instead of dylibs (experimental)" OFF)
    SET(OSG_COMPILE_FRAMEWORKS_INSTALL_NAME_DIR "@executable_path/../Frameworks" CACHE STRING "install name dir for compiled frameworks")
ELSE()
    # Non-Apple: Find OpenGL
    
    # Support EGL on Windows
    IF(OSG_WINDOWING_SYSTEM STREQUAL "Win32" AND (OSG_GLES2_AVAILABLE OR OSG_GLES3_AVAILABLE ) )
        FIND_PACKAGE(EGL)
    
        set(OSG_USE_EGL ON)

        IF(OSG_GLES3_AVAILABLE)
            FIND_PATH(OPENGL_INCLUDE_DIR GLES3/gl3.h)
        ENDIF()
        IF(OSG_GLES2_AVAILABLE)
            FIND_PATH(OPENGL_INCLUDE_DIR GLES2/gl2.h)
        ENDIF()
        FIND_LIBRARY(OPENGL_gl_LIBRARY "libGLESv2.dll")
    ELSE()
        # Non-Apple: Find OpenGL
        FIND_PACKAGE(OpenGL)
    ENDIF()

ENDIF()
ENDIF()

INCLUDE(OsgMacroUtils)


IF(UNIX AND NOT ANDROID)
    # Not sure what this will do on Cygwin and Msys
    # Also, remember OS X X11 is a user installed option so it may not exist.
    FIND_PACKAGE(X11)
    # Some Unicies need explicit linkage to the Math library or the build fails.
    FIND_LIBRARY(MATH_LIBRARY m)

    FIND_LIBRARY(DL_LIBRARY dl)
    IF(NOT DL_LIBRARY)
        SET(DL_LIBRARY "") # change from NOTFOUND to empty when passed to linker
    ENDIF()

    IF( CMAKE_SYSTEM MATCHES "Linux" )
        FIND_LIBRARY( RT_LIBRARY rt )
    ENDIF( CMAKE_SYSTEM MATCHES "Linux" )

ENDIF()


# Make the headers visible to everything
IF(NOT ${PROJECT_BINARY_DIR} EQUAL ${PROJECT_SOURCE_DIR})
   INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR}/include)
ENDIF()

INCLUDE_DIRECTORIES(${osgThreeJSX_SOURCE_DIR}/include/)

INCLUDE_DIRECTORIES(SYSTEM ${OPENGL_INCLUDE_DIR})

# Common global definitions
#ADD_DEFINITIONS(-D)
# Platform specific definitions


IF(WIN32 AND NOT ANDROID)

    IF(MSVC)
        # This option is to enable the /MP switch for Visual Studio 2005 and above compilers
        OPTION(WIN32_USE_MP "Set to ON to build OSGTHREEJSX with the /MP option (Visual Studio 2005 and above)." OFF)
        MARK_AS_ADVANCED(WIN32_USE_MP)
        IF(WIN32_USE_MP)
            SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
        ENDIF(WIN32_USE_MP)

        # turn off various warnings
        # foreach(warning 4244 4251 4267 4275 4290 4786 4305 4996)
        #     SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd${warning}")
        # endforeach(warning)

        # This option is to enable the /DYNAMICBASE switch
        # It is used to workaround a bug in Windows 7 when linking in release, which results in corrupt
        # binaries. See this page for details: http://www.wintellect.com/CS/blogs/jrobbins/archive/2009/01/24/the-case-of-the-corrupt-pe-binaries.aspx
        OPTION(WIN32_USE_DYNAMICBASE "Set to ON to build OSGTHREEJSX with the /DYNAMICBASE option to work around a bug when linking release executables on Windows 7." OFF)
        MARK_AS_ADVANCED(WIN32_USE_DYNAMICBASE)
        IF(WIN32_USE_DYNAMICBASE)
            SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /DYNAMICBASE")
        ENDIF(WIN32_USE_DYNAMICBASE)

        # More MSVC specific compilation flags
        ADD_DEFINITIONS(-D_SCL_SECURE_NO_WARNINGS)
        ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE)

        OPTION(MSVC_DISABLE_CHECKED_ITERATORS "Set to ON to disable Visual C++ checked iterators. If you do this you must ensure that every other project in your solution and all dependencies are compiled with _SECURE_SCL=0." OFF)
        MARK_AS_ADVANCED(MSVC_DISABLE_CHECKED_ITERATORS)
        IF(MSVC_DISABLE_CHECKED_ITERATORS)
            ADD_DEFINITIONS(-D_SECURE_SCL=0)
        ENDIF(MSVC_DISABLE_CHECKED_ITERATORS)

        OPTION(MSVC_USE_DEFAULT_STACK_SIZE "Set to ON to use the default Visual C++ stack size. CMake forces a high stack size by default, which can cause problems for applications with large number of threads." OFF)
        MARK_AS_ADVANCED(MSVC_USE_DEFAULT_STACK_SIZE)
        IF(MSVC_USE_DEFAULT_STACK_SIZE)
            STRING(REGEX REPLACE "/STACK:[0-9]+" "" CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")
            STRING(REGEX REPLACE "/STACK:[0-9]+" "" CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}")
            STRING(REGEX REPLACE "/STACK:[0-9]+" "" CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS}")
        ENDIF(MSVC_USE_DEFAULT_STACK_SIZE)
        get_filename_component( CMAKE_MAKE_PROGRAM_NAME ${CMAKE_MAKE_PROGRAM} NAME)
        IF (CMAKE_MAKE_PROGRAM_NAME STREQUAL "VCExpress.exe")
            OPTION(MSVC_BUILD_USE_SOLUTION_FOLDERS "Enable project grouping in VS - VCExpress detected, not supported in VCExpress )" OFF)
        ELSE()
            OPTION(MSVC_BUILD_USE_SOLUTION_FOLDERS "Enable project grouping in VS" ON)
        ENDIF()
        SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ${MSVC_BUILD_USE_SOLUTION_FOLDERS})
    ENDIF()

    #needed for net plugin
    # Both Cygwin and Msys need -DNOMINMAX ???
    IF(UNIX)
        ADD_DEFINITIONS(-DNOMINMAX)
    ENDIF()

    IF(MSVC)
        OPTION(OSG_MSVC_VERSIONED_DLL "Set to ON to build OpenSceneGraph with versioned dll names" ON)
        MARK_AS_ADVANCED(OSG_MSVC_VERSIONED_DLL)
        OPTION(OSG_MSVC_DEBUG_INCREMENTAL_LINK "Set to OFF to build OpenSceneGraph without incremental linking in debug (release is off by default)" ON)
        MARK_AS_ADVANCED(OSG_MSVC_DEBUG_INCREMENTAL_LINK)
        IF(NOT OSG_MSVC_DEBUG_INCREMENTAL_LINK)
            SET(CMAKE_MODULE_LINKER_FLAGS_DEBUG "/debug /INCREMENTAL:NO")
            SET(CMAKE_SHARED_LINKER_FLAGS_DEBUG "/debug /INCREMENTAL:NO")
            SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "/debug /INCREMENTAL:NO")
        ENDIF(NOT OSG_MSVC_DEBUG_INCREMENTAL_LINK)
    ENDIF(MSVC)
ENDIF(WIN32 AND NOT ANDROID)


################################################################################
# Set Version Info resource file
IF(MSVC)
    SET(OSGTHREEJSX_VERSIONINFO_RC "${PROJECT_BINARY_DIR}/PlatformSpecifics/Windows/osgThreeJSXVersionInfo.rc")
    CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/PlatformSpecifics/Windows/osgThreeJSXVersionInfo.rc.in"
                   "${OSGTHREEJSX_VERSIONINFO_RC}")
ENDIF()

################################################################################
# Optional build components

# osgThreeJSX examples
OPTION(BUILD_OSGTHREEJSX_EXAMPLES "Enable to build osgThreeJSX examples" ON)

# OSG Plugins disable option for apple build on travis ci test - full build job runs over time limit of 50 min.
OPTION(BUILD_OSGTHREEJSX_PLUGINS "Build OSG Plugins - Disable for compile testing examples on a time limit" ON)
mark_as_advanced(BUILD_OSGTHREEJSX_PLUGINS)

################################################################################
# Create bin and lib directories if required

IF("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
   FILE(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/bin ${CMAKE_BINARY_DIR}/lib ${CMAKE_BINARY_DIR}/lib/${OSG_PLUGINS})
ENDIF()


################################################################################
# Installation stuff

SET(CMAKE_DEBUG_POSTFIX "d" CACHE STRING "add a postfix, usually d on windows")
SET(CMAKE_RELEASE_POSTFIX "" CACHE STRING "add a postfix, usually empty on windows")
SET(CMAKE_RELWITHDEBINFO_POSTFIX "rd" CACHE STRING "add a postfix, usually empty on windows")
SET(CMAKE_MINSIZEREL_POSTFIX "s" CACHE STRING "add a postfix, usually empty on windows")

# Correct any incorrect case usage in CMAKE_BUILD_TYPE
IF     (CMAKE_BUILD_TYPE MATCHES "release" OR CMAKE_BUILD_TYPE MATCHES "RELEASE")
    SET(CMAKE_BUILD_TYPE "Release")
ELSEIF (CMAKE_BUILD_TYPE MATCHES "minsizerel" OR CMAKE_BUILD_TYPE MATCHES "MINSIZEREL")
    SET(CMAKE_BUILD_TYPE "MinSizeRel")
ELSEIF (CMAKE_BUILD_TYPE MATCHES "relwithdebinfo" OR CMAKE_BUILD_TYPE MATCHES "RELWITHDEBINFO")
    SET(CMAKE_BUILD_TYPE "RelWithDebInfo")
ELSEIF (CMAKE_BUILD_TYPE MATCHES "debug" OR CMAKE_BUILD_TYPE MATCHES "DEBUG")
    SET(CMAKE_BUILD_TYPE "Debug")
ENDIF()

# Set the build postfix extension according to what configuration is being built.
IF (CMAKE_BUILD_TYPE MATCHES "Release")
    SET(CMAKE_BUILD_POSTFIX "${CMAKE_RELEASE_POSTFIX}")
ELSEIF (CMAKE_BUILD_TYPE MATCHES "MinSizeRel")
    SET(CMAKE_BUILD_POSTFIX "${CMAKE_MINSIZEREL_POSTFIX}")
ELSEIF(CMAKE_BUILD_TYPE MATCHES "RelWithDebInfo")
    SET(CMAKE_BUILD_POSTFIX "${CMAKE_RELWITHDEBINFO_POSTFIX}")
ELSEIF(CMAKE_BUILD_TYPE MATCHES "Debug")
    SET(CMAKE_BUILD_POSTFIX "${CMAKE_DEBUG_POSTFIX}")
ELSE()
    SET(CMAKE_BUILD_POSTFIX "")
ENDIF()

IF(UNIX AND NOT WIN32)
  SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")
  SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -D_DEBUG")
ENDIF()

IF(CYGWIN)
  SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")
  SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -D_DEBUG")
ENDIF()


# Here we apparently do some funky stuff with making the bin/ and lib/
# folders which is probably needed to work around a very old CMake bug?

#SET(OUTPUT_BINDIR ${PROJECT_BINARY_DIR}/bin/${CMAKE_SYSTEM_NAME})
SET(OUTPUT_BINDIR ${PROJECT_BINARY_DIR}/bin)
MAKE_DIRECTORY(${OUTPUT_BINDIR})
IF(MSVC AND NOT MSVC_IDE)
    MAKE_DIRECTORY(${OUTPUT_BINDIR}/${OSG_PLUGINS})
ENDIF(MSVC AND NOT MSVC_IDE)

#SET(OUTPUT_LIBDIR ${PROJECT_BINARY_DIR}/lib/${CMAKE_SYSTEM_NAME})
SET(OUTPUT_LIBDIR ${PROJECT_BINARY_DIR}/lib)
MAKE_DIRECTORY(${OUTPUT_LIBDIR})
IF(NOT MSVC OR MSVC_IDE)
    MAKE_DIRECTORY(${OUTPUT_LIBDIR}/${OSG_PLUGINS})
ENDIF(NOT MSVC OR MSVC_IDE)

# CMake >= 2.8.1 changed the output directory algorithm (See doc).
# Here we also set per-configuration directories (CMAKE_*_OUTPUT_DIRECTORY_<CONFIG>), or else binaries are generated in /bin/Debug and /bin/Release, etc. with MSVC and Xcode.
# (Doc reads "multi-configuration generators (VS, Xcode) do NOT append a per-configuration subdirectory to the specified directory").
# The workaround for 2.6.x (adding "../" as an output prefix for each target) seem to have no effect in >=2.8.1, so there is no need to change this.
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_LIBDIR})
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_BINDIR})
IF(WIN32)
    SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_BINDIR})
ELSE(WIN32)
    SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_LIBDIR})
ENDIF(WIN32)

BUILDER_VERSION_GREATER(2 8 0)
IF(VALID_BUILDER_VERSION)  # If CMake >= 2.8.1
    FOREACH(CONF ${CMAKE_CONFIGURATION_TYPES})        # For each configuration (Debug, Release, MinSizeRel... and/or anything the user chooses)
        STRING(TOUPPER "${CONF}" CONF)                # Go uppercase (DEBUG, RELEASE...)
        SET("CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CONF}" "${OUTPUT_LIBDIR}")
        SET("CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONF}" "${OUTPUT_BINDIR}")
        IF(WIN32)
            SET("CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONF}" "${OUTPUT_BINDIR}")
        ELSE()
            SET("CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONF}" "${OUTPUT_LIBDIR}")
        ENDIF()
    ENDFOREACH()
ENDIF(VALID_BUILDER_VERSION)

SET(INSTALL_BINDIR OSGTHREEJSX/bin)
SET(INSTALL_INCDIR OSGTHREEJSX/include)
SET(INSTALL_LIBDIR OSGTHREEJSX/lib)
SET(INSTALL_DOCDIR OSGTHREEJSX/doc)

################################################################################
# User Options


# Expose CMAKE_INCLUDE_PATH and CMAKE_LIBARY_PATH to the GUI so users
# may set these values without needing to manipulate the environment.
SET(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} CACHE STRING "You may add additional search paths here. Use ; to separate multiple paths.")
SET(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} CACHE STRING "You may add additional search paths here. Use ; to separate multiple paths.")
# We are proposing that a new variable called CMAKE_PREFIX_PATH be introduced
# to CMake to compliment CMAKE_INCLUDE_PATH and CMAKE_LIBRARY_PATH.
# A formal feature request has been submited to CMake, Bug #4947.
# It is intended for those users who have common prefixes for their INCLUDE
# and LIBRARY locations. So if users have headers in /usr/local/include
# and libraries in /usr/local/lib, the common prefix is /usr/local.
# It should also cover the case where headers and libraries are
# in the same directory.
# Our proposal expects that FIND_* commands will automatically search for
# CMAKE_PREFIX_PATH right after CMAKE_INCLUDE_PATH or CMAKE_LIBRARY_PATH.
# Obviously, since CMake does not currently support this, we must write
# our Find*.cmake modules to explicitly support this. Otherwise, this variable
# will have no impact.
# This is unofficial so this may be removed or changed at anytime.
SET(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} CACHE STRING "(EXPERIMENTAL) You may add additional search paths here. Use ; to separate multiple paths.")


# This is for an advanced option to give aggressive warnings
# under different compilers. If yours is not implemented, this option
# will not be made available.
IF(CMAKE_COMPILER_IS_GNUCXX)
    # To be complete, we might also do GNUCC flags,
    # but everything here is C++ code.
    # -Wshadow and -Woverloaded-virtual are also interesting flags, but OSG
    # returns too many hits.
    # FYI, if we do implement GNUCC, then -Wmissing-prototypes in another
    # interesting C-specific flag.
    # Also, there is a bug in gcc 4.0. Under C++, -pedantic will create
    # errors instead of warnings for certain issues, including superfluous
    # semicolons and commas, and the use of long long. -fpermissive seems
    # to be the workaround.
    SET(OSGTHREEJSX_AGGRESSIVE_WARNING_FLAGS -Wall -Wparentheses -Wno-long-long -Wno-import -pedantic -Wreturn-type -Wmissing-braces -Wunknown-pragmas -Wunused)

    MESSAGE( STATUS "g++ version ${CMAKE_CXX_COMPILER_VERSION} ")
    IF(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 4.6)
        SET(OSGTHREEJSX_AGGRESSIVE_WARNING_FLAGS ${OSGTHREEJSX_AGGRESSIVE_WARNING_FLAGS} -Wmaybe-uninitialized -Wextra)
        IF(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 4.9)
            SET(OSGTHREEJSX_AGGRESSIVE_WARNING_FLAGS ${OSGTHREEJSX_AGGRESSIVE_WARNING_FLAGS} -Wshadow)
            IF(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 6.0)
                # -Wmisleading-indentation (in -Wall) generates less warnings when interpreting tab as 4 spaces instead of the default of 8
                # SET(OSGTHREEJSX_AGGRESSIVE_WARNING_FLAGS ${OSGTHREEJSX_AGGRESSIVE_WARNING_FLAGS} -ftabstop=4)
                SET(OSGTHREEJSX_AGGRESSIVE_WARNING_FLAGS ${OSGTHREEJSX_AGGRESSIVE_WARNING_FLAGS} -Wno-misleading-indentation)
            ENDIF()
        ENDIF()
    ENDIF()

    # Previous included -Wformat=2 in OSGTHREEJSX_AGGRESSIVE_WARNING_FLAGS but had to remove it due to standard library errors

ELSEIF(MSVC)
    #disable specific warning level 4 warnings:
    #C4100 'identifier' : unreferenced formal parameter
    #C4127 Error Message conditional expression is constant
    #C4706 assignment within conditional expression
    #C4589: Constructor of abstract class 'osgGA::CameraManipulator' ignores initializer for virtual base class 'osg::Object'
    SET(OSGTHREEJSX_AGGRESSIVE_WARNING_FLAGS /W4 /wd4589 /wd4706 /wd4127 /wd4100)
ELSEIF(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    SET(OSGTHREEJSX_AGGRESSIVE_WARNING_FLAGS  -Wall -Wparentheses -Wno-long-long -Wno-import -pedantic -Wreturn-type -Wmissing-braces -Wunknown-pragmas -Wunused -Wno-overloaded-virtual)

    IF (APPLE)
        SET(OSG_CXX_LANGUAGE_STANDARD "C++11" CACHE STRING "set the c++ language standard (C++98 / GNU++98 / C++11) for OSG" )
        MARK_AS_ADVANCED(OSG_CXX_LANGUAGE_STANDARD)

        # remove existing flags
        REMOVE_CXX_FLAG(-std=c++98)
        REMOVE_CXX_FLAG(-std=gnu++98)
        REMOVE_CXX_FLAG(-std=c++11)
        REMOVE_CXX_FLAG(-stdlib=libstdc++)
        REMOVE_CXX_FLAG(-stdlib=libc++)

        IF(${OSG_CXX_LANGUAGE_STANDARD} STREQUAL "c++98" OR ${OSG_CXX_LANGUAGE_STANDARD} STREQUAL "C++98")
            set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++98")
            set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libstdc++")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++98 -stdlib=libstdc++")
        ELSEIF(${OSG_CXX_LANGUAGE_STANDARD} STREQUAL "gnu++98" OR ${OSG_CXX_LANGUAGE_STANDARD} STREQUAL "GNU++98")
            set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "gnu++98")
            set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libstdc++")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++98 -stdlib=libstdc++")
        ELSE()
            set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++11")
            set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++")
        ENDIF()

        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-overloaded-virtual -Wno-conversion")
        set(WARNING_CFLAGS "")
    ENDIF()
ENDIF()

# This part is for the CMake menu option to toggle the warnings on/off.
# This will only be made available if we set values for OSGTHREEJSX_AGGRESSIVE_WARNING_FLAGS.
IF(OSGTHREEJSX_AGGRESSIVE_WARNING_FLAGS)

    SET(OSGTHREEJSX_AGGRESSIVE_WARNING_FLAGS ${OSGTHREEJSX_AGGRESSIVE_WARNING_FLAGS} CACHE STRING "Compiler flags to use when OSG_AGGRESSIVE_WARNINGS is enabled." FORCE)

    IF (APPLE)
        SET(DEFAULT_USE_AGGRESSIVE_WARNINGS OFF)
    ELSE()
        SET(DEFAULT_USE_AGGRESSIVE_WARNINGS ON)
    ENDIF()

    OPTION(OSG_AGGRESSIVE_WARNINGS "Enable to activate aggressive warnings" ${DEFAULT_USE_AGGRESSIVE_WARNINGS})

    IF(OSG_AGGRESSIVE_WARNINGS)
        # Add flags defined by OSGTHREEJSX_AGGRESSIVE_WARNING_FLAGS if they aren't already there
        FOREACH(flag ${OSGTHREEJSX_AGGRESSIVE_WARNING_FLAGS})
            IF(NOT CMAKE_CXX_FLAGS MATCHES "${flag}")
                SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}")
            ENDIF()
        ENDFOREACH()
    ELSE()
        # Remove all flags considered aggressive
        FOREACH(flag ${OSGTHREEJSX_AGGRESSIVE_WARNING_FLAGS})
            STRING(REGEX REPLACE "${flag}" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
        ENDFOREACH()
    ENDIF()
ENDIF()


# Dynamic vs Static Linking
OPTION(DYNAMIC_OSGTHREEJSX "Set to ON to build OSGTHREEJSX for dynamic linking.  Use OFF for static." ON)
IF (DYNAMIC_OSGTHREEJSX)
    SET(OSGTHREEJSX_USER_DEFINED_DYNAMIC_OR_STATIC "SHARED")
ELSE ()
    SET(OSGTHREEJSX_USER_DEFINED_DYNAMIC_OR_STATIC "STATIC")
ENDIF()

# osgThressJSX Core
ADD_SUBDIRECTORY(src)

IF (BUILD_OSGTHREEJSX_EXAMPLES)
    ADD_SUBDIRECTORY(examples)
ENDIF()