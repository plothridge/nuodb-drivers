# PHP Driver for NuoDB #

This exposes a PHP interface which is similar to the NuoDB C language 
interface.

This interface is written as a PHP extension.  It uses CMake to build. 
It has been tested using g++ on Linux 64-bit and Visual Studio 2008 
on Windows 32-bits.

## PREREQUISITES ##

TODO: TOM

Please document how to build this; apparently there are some prerequisites.
What in addition to these are required?

  sudo apt-get install php5
  sudo apt-get install php5-dev

## BUILDING ##

Two scripts are provided: do_win32.bat and do_linux64. These scripts will
clean out old files, run cmake, then make, then install the PHP extension,
and then run the tests/simpletest.php

For Windows, from Visual Studio 2008 (or newer) command prompt, run:

```bash
do_win32.bat
```

For Linux run:

```bash
./do_linux64
```

## INSTALL ##

Installation happens automatically when you run the build script.

TBD Tom - please document running CPack and document how to install the
built binaries.

## QUICK START ##

To run simpletest.php, you should have a nuoagent, nuodb archive-manager,
and a nuodb transaction processing node running on test@localhost which 
has the following example hockey db:

```sql
create schema hockey
use hockey
create table players (lastname char(50), firstname char(50))
insert into players(lastname, firstname) values('thomas', 'tim')
```

## LICENSE ##

This module is released under the [NUODB License] [license].

Related Pages:

[homepage]: http://www.nuodb.com
[license]: https://github.com/nuodb/nuodb-drivers/blob/master/LICENSE
