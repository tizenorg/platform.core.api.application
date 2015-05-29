CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
SET(fw_name "capi-appfw-application")

PROJECT(${fw_name})

SET(CMAKE_INSTALL_PREFIX /usr)
SET(PREFIX ${CMAKE_INSTALL_PREFIX})

SET(INC_DIR include)
INCLUDE_DIRECTORIES(${INC_DIR})

SET(requires "dlog bundle appcore-common appcore-efl aul ail appsvc elementary capi-base-common alarm-service sqlite3 libtzplatform-config pkgmgr-info glib-2.0 capi-system-info capi-system-system-settings")
SET(pc_requires "capi-base-common vconf-internal-keys capi-appfw-alarm capi-appfw-app-control capi-appfw-app-common capi-appfw-preference capi-appfw-event")

INCLUDE(FindPkgConfig)
pkg_check_modules(${fw_name} REQUIRED ${requires})
FOREACH(flag ${${fw_name}_CFLAGS})
    SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS} -fPIC ")
SET(CMAKE_C_FLAGS_DEBUG "-O0 -g -Wall -Werror")

IF("${ARCH}" STREQUAL "arm")
    ADD_DEFINITIONS("-DTARGET")
ENDIF("${ARCH}" STREQUAL "arm")

ADD_DEFINITIONS("-DPREFIX=\"${CMAKE_INSTALL_PREFIX}\"")
ADD_DEFINITIONS("-DSLP_DEBUG")

SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed -Wl,--rpath=${LIB_INSTALL_DIR}")

aux_source_directory(src SOURCES)
ADD_LIBRARY(${fw_name} SHARED ${SOURCES})

ADD_SUBDIRECTORY(app_control)
ADD_SUBDIRECTORY(app_common)
ADD_SUBDIRECTORY(alarm)
ADD_SUBDIRECTORY(preference)
ADD_SUBDIRECTORY(event)

TARGET_LINK_LIBRARIES(${fw_name} capi-appfw-app-control capi-appfw-app-common capi-appfw-alarm capi-appfw-preference capi-appfw-event ${${fw_name}_LDFLAGS})

SET_TARGET_PROPERTIES(${fw_name}
     PROPERTIES
     VERSION ${FULLVER}
     SOVERSION ${MAJORVER}
     CLEAN_DIRECT_OUTPUT 1
)

INSTALL(TARGETS ${fw_name} DESTINATION ${LIB_INSTALL_DIR})
INSTALL(
        DIRECTORY ${INC_DIR}/ DESTINATION include/appfw
        FILES_MATCHING
        PATTERN "*_private.h" EXCLUDE
        PATTERN "${INC_DIR}/*.h"
        )

SET(PC_NAME ${fw_name})
SET(PC_REQUIRED ${pc_requires})
SET(PC_LDFLAGS -l${fw_name})

CONFIGURE_FILE(
    capi-appfw-module.pc.in
    ${CMAKE_CURRENT_SOURCE_DIR}/${fw_name}.pc
    @ONLY
)
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/${fw_name}.pc DESTINATION ${LIB_INSTALL_DIR}/pkgconfig)

IF(UNIX)

ADD_CUSTOM_TARGET (distclean @echo cleaning for source distribution)
ADD_CUSTOM_COMMAND(
        DEPENDS clean 
        COMMENT "distribution clean"
        COMMAND find
        ARGS    . 
        -not -name config.cmake -and \(
        -name tester.c -or
        -name Testing -or
        -name CMakeFiles -or
        -name cmake.depends -or
        -name cmake.check_depends -or
        -name CMakeCache.txt -or
        -name cmake.check_cache -or
        -name *.cmake -or
        -name Makefile -or
        -name core -or
        -name core.* -or
        -name gmon.out -or
        -name install_manifest.txt -or
        -name *.pc -or
        -name *~ \)
        | grep -v TC | xargs rm -rf
        TARGET  distclean
        VERBATIM
)

ENDIF(UNIX)

