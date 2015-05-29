CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
SET(fw_name "capi-appfw-preference")

PROJECT(${fw_name})

SET(CMAKE_INSTALL_PREFIX /usr)
SET(PREFIX ${CMAKE_INSTALL_PREFIX})

SET(INC_DIR ${CMAKE_SOURCE_DIR}/include)
INCLUDE_DIRECTORIES(${INC_DIR})

SET(requires "dlog capi-base-common sqlite3")
SET(pc_requires "capi-base-common capi-appfw-app-common")

INCLUDE(FindPkgConfig)
pkg_check_modules(${fw_name} REQUIRED ${requires})
FOREACH(flag ${${fw_name}_CFLAGS})
    SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS} -fPIC -Wall -Werror")
SET(CMAKE_C_FLAGS_DEBUG "-O0 -g")

IF("${ARCH}" STREQUAL "arm")
    ADD_DEFINITIONS("-DTARGET")
ENDIF("${ARCH}" STREQUAL "arm")

ADD_DEFINITIONS("-DPREFIX=\"${CMAKE_INSTALL_PREFIX}\"")
ADD_DEFINITIONS("-DSLP_DEBUG")

SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed -Wl,--rpath=${LIB_INSTALL_DIR}")

add_library(${fw_name} SHARED
		preference.c
		preference_inoti.c
		)

TARGET_LINK_LIBRARIES(${fw_name} capi-appfw-app-common ${${fw_name}_LDFLAGS})

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
    ${CMAKE_SOURCE_DIR}/capi-appfw-module.pc.in
    ${CMAKE_SOURCE_DIR}/${fw_name}.pc
    @ONLY
)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/${fw_name}.pc DESTINATION ${LIB_INSTALL_DIR}/pkgconfig)


