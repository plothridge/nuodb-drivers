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
cmake -G "Visual Studio 9 2008" -D NUODB_ROOT=%NUODB_ROOT% -D PHP_ROOT=%PHP_INSTALL_DIR% -D PHP_INCLUDE=%PHP_INSTALL_DIR%/SDK/include .
devenv NUOPHP.sln /build Debug /project php_nuodb
devenv NUOPHP.sln /build RelWithDebInfo /project php_nuodb
copy /Y Debug\*.* %PHP_INSTALL_DIR%\debug
copy /Y RelWithDebInfo\*.* %PHP_INSTALL_DIR%
php tests\simpletest.php

