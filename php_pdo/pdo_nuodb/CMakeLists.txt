# Copyright (c) 2012, NuoDB, Inc.
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
# 
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the name of NuoDB, Inc. nor the names of its contributors may
#       be used to endorse or promote products derived from this software
#       without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL NUODB, INC. BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
# OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
# LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
# OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#
# PHP PDO Native Database Driver for NuoDB
# 

PROJECT (NuoPhpPdo)
CMAKE_MINIMUM_REQUIRED (VERSION 2.8 FATAL_ERROR)

INCLUDE (CheckIncludeFile)
INCLUDE (CheckIncludeFiles)
INCLUDE (CheckFunctionExists)
INCLUDE (CheckTypeSize)
INCLUDE (CTest)
INCLUDE (InstallRequiredSystemLibraries)

EXECUTE_PROCESS(COMMAND git rev-list HEAD -n1 --abbrev-commit
    OUTPUT_STRIP_TRAILING_WHITESPACE
    OUTPUT_VARIABLE NUODB_PHPPDO_SCM_REVISION)

SET (CPACK_PACKAGE_NAME "NuoPhpPdo")
SET (CPACK_PACKAGE_VENDOR "NuoDB, Inc.")
SET (CPACK_PACKAGE_VERSION_MAJOR "1")
SET (CPACK_PACKAGE_VERSION_MINOR "0")
SET (CPACK_PACKAGE_VERSION_PATCH "0")
SET (CPACK_PACKAGE_VENDOR "NuoDB, Inc.")
SET (CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
SET (CPACK_PACKAGE_DESCRIPTION_SUMMARY "A PHP PDO Native Database Driver for NuoDB")

SET(CPACK_GENERATOR "TGZ;ZIP")
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "Robert Buck") #required

INCLUDE (CPack)

IF (NOT CMAKE_BUILD_TYPE)
    SET (CMAKE_BUILD_TYPE Release CACHE STRING
        "Choose the type of build, options are Debug Release"
        FORCE)
ENDIF (NOT CMAKE_BUILD_TYPE)

FIND_PATH (PHP_INCLUDE_DIR main/php.h
    PATHS $ENV{PHP_INCLUDE_DIR}
    DOC "PHP includes directory")

IF (PHP_INCLUDE_DIR)
    MESSAGE (STATUS "PHP includes directory found: " ${PHP_INCLUDE_DIR})
ELSE (PHP_INCLUDE_DIR)
    MESSAGE (FATAL_ERROR "PHP includes directory not found; please set PHP_INCLUDE_DIR variable")
ENDIF (PHP_INCLUDE_DIR)

FIND_PATH (NUODB_INCLUDE_DIR Connection.h
    PATHS $ENV{NUODB_INCLUDE_DIR}
    DOC "NuoDB includes directory")

IF (NUODB_INCLUDE_DIR)
    MESSAGE (STATUS "NuoDB includes directory found: " ${NUODB_INCLUDE_DIR})
ELSE (NUODB_INCLUDE_DIR)
    MESSAGE (FATAL_ERROR "NuoDB includes directory not found; please set NUODB_INCLUDE_DIR variable")
ENDIF (NUODB_INCLUDE_DIR)

FIND_LIBRARY (NUODB_REMOTE_LIBRARY
    NAMES NuoRemote libNuoRemote
    PATHS $ENV{NUODB_LIB_DIR}
    DOC "NuoDB remote library to link against")
             
IF (NUODB_REMOTE_LIBRARY)
    MESSAGE (STATUS "NuoRemote library found: " ${NUODB_REMOTE_LIBRARY})
ELSE (NUODB_REMOTE_LIBRARY)
    MESSAGE (FATAL_ERROR "NuoRemote library not found; please set NUODB_LIB_DIR variable")
ENDIF (NUODB_REMOTE_LIBRARY)

INCLUDE_DIRECTORIES (
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${NUODB_INCLUDE_DIR}
    ${PHP_INCLUDE_DIR}
    ${PHP_INCLUDE_DIR}/main
    ${PHP_INCLUDE_DIR}/TSRM
    ${PHP_INCLUDE_DIR}/Zend
    ${PHP_INCLUDE_DIR}/ext
    ${PHP_INCLUDE_DIR}/ext/date/lib
    )

SET (NUODB_PHPPDO_SOURCES
    nuodb_driver.cpp
    pdo_nuodb.cpp
    nuodb_statement.cpp
    php_pdo_nuodb_cpp_int.cpp
    php_pdo_nuodb_cpp_int.h
    php_pdo_nuodb_int.h
    php_pdo_nuodb.h
    )

IF (CMAKE_GENERATOR MATCHES "Unix Makefiles" OR CMAKE_GENERATOR MATCHES "Xcode")
    ADD_DEFINITIONS (-DPHP_ATOM_INC)
    ADD_DEFINITIONS (-DHAVE_CONFIG_H)
    ADD_DEFINITIONS (-DHAVE_DLFCN_H=1)
    ADD_DEFINITIONS (-Wno-write-strings)
    ADD_LIBRARY (NuoPhpPdo SHARED ${NUODB_PHPPDO_SOURCES})
    LINK_DIRECTORIES (${NUODB_LIB_DIR})
    TARGET_LINK_LIBRARIES (NuoPhpPdo ${NUODB_REMOTE_LIBRARY})
    SET_TARGET_PROPERTIES (NuoPhpPdo PROPERTIES VERSION ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH} SOVERSION ${CPACK_PACKAGE_VERSION_MAJOR})
    SET_TARGET_PROPERTIES(NuoPhpPdo PROPERTIES PREFIX "")
    SET_TARGET_PROPERTIES (NuoPhpPdo PROPERTIES OUTPUT_NAME php_pdo_nuodb)
ENDIF (CMAKE_GENERATOR MATCHES "Unix Makefiles" OR CMAKE_GENERATOR MATCHES "Xcode")

IF(CMAKE_GENERATOR MATCHES "Visual Studio 7" OR
    CMAKE_GENERATOR MATCHES "Visual Studio 8" OR
    CMAKE_GENERATOR MATCHES "Visual Studio 9" OR
    CMAKE_GENERATOR MATCHES "Visual Studio 10")
    ADD_DEFINITIONS (-DZTS=1 -DHAVE_CONFIG_H -DZEND_DEBUG=0 -DCOMPILE_DL_NUODB -DPHP_WIN32 -DZEND_WIN32)
    ADD_LIBRARY (php5ts SHARED IMPORTED)
    SET_PROPERTY (TARGET php5ts PROPERTY IMPORTED_LOCATION ${PHP_ROOT}/php5ts.dll)
    SET_PROPERTY (TARGET php5ts PROPERTY IMPORTED_IMPLIB ${PHP_ROOT}/SDK/lib/php5ts.lib)
    ADD_LIBRARY (nuoremote SHARED IMPORTED)
    SET_PROPERTY (TARGET nuoremote PROPERTY IMPORTED_LOCATION ${NUODB_ROOT}/bin/NuoRemote.dll)
    SET_PROPERTY (TARGET nuoremote PROPERTY IMPORTED_IMPLIB ${NUODB_ROOT}/lib/NuoRemote.lib)
    ADD_LIBRARY (NuoPhpPdo SHARED ${NUODB_PHPPDO_SOURCES})
    TARGET_LINK_LIBRARIES (NuoPhpPdo nuoremote php5ts)
ENDIF(CMAKE_GENERATOR MATCHES "Visual Studio 7" OR
      CMAKE_GENERATOR MATCHES "Visual Studio 8" OR
      CMAKE_GENERATOR MATCHES "Visual Studio 9" OR
      CMAKE_GENERATOR MATCHES "Visual Studio 10")

INSTALL(TARGETS NuoPhpPdo
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    )

ADD_TEST(001.phpt tests/001.phpt)
