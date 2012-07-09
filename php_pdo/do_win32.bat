set PHP_INSTALL_DIR=C:\php
set NUODB_ROOT=C:/PROGRA~1/NuoDB

set PATH=%PHP_INSTALL_DIR%\SDK;%PHP_INSTALL_DIR%;%PATH%
set LIB=%PHP_INSTALL_DIR%\SDK\lib;%LIB%
set PHPRC=%PHP_INSTALL_DIR%\php.ini

del /Q CMakeCache.txt
rmdir /Q /S CMakeFiles
rmdir /Q /S Debug 
rmdir /Q /S RelWithDebInfo
rmdir /Q /S php_nuodb.dir
del /Q cmake_install.cmake
del /Q NUOPHP.suo
cmake -G "Visual Studio 9 2008" -D NUODB_ROOT=%NUODB_ROOT% -D NUODB_INCLUDE_DIR=%NUODB_ROOT%/include -D NUODB_LIB_DIR=%NUODB_ROOT%/lib -D NUODB_REMOTE_LIBRARY=%NUODB_ROOT%/lib -D PHP_ROOT=%PHP_INSTALL_DIR% -D PHP_INCLUDE_DIR=%PHP_INSTALL_DIR%/SDK/include .
devenv NuoPhpPdo.sln /build Debug /project NuoPhpPdo
devenv NuoPhpPdo.sln /build RelWithDebInfo /project NuoPhpPdo
copy /Y Debug\*.* %PHP_INSTALL_DIR%\debug
copy /Y RelWithDebInfo\*.* %PHP_INSTALL_DIR%
php tests\simpletest.php

