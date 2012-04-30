# Perl DBI DBD Driver for NuoDB #

This project implements a NuoDB DBI driver for Perl

## LIMITATIONS ##

The following features have been implemented:

* DSNs in the form dbi:NuoDB:database@host:port
* Schema selection via the "schema" handle attribute
* Transactions, commit(), rollback() and the AutoCommit handle attribute
* prepare(), execute(), fetch() as well as the combined convenience functions such as selectall_arrayref()

The following has not been implemented yet:
* All result values are returned as strings regardless of the original data type
* Metadata methods have not been implemented
* "?"-style placeholders, bind variables, etc. are not supported
* Unicode and strings containing "\0" may be truncated
* Windows has not been tested

## EXAMPLE ##

```bash
export LD_PRELOAD=/opt/nuodb/lib64/libNuoRemote.so
```

```perl
use DBI;
my $dbh = DBI->connect("dbi:NuoDB:".$database.'@'.$host':'.$port, $username, $password, {schema => $schema});
my $sth = $dbh->prepare("SELECT 'one' FROM DUAL");
$sth->execute();
my ($value) = $sth->fetchrow_array();
```

## PREREQUISITES ##

* A recent version of DBI. To upgrade DBI:

```bash
sudo perl -MCPAN -e 'force install DBI'
```

## BUILDING AND INSTALLATION ##

```bash
export LD_PRELOAD=/opt/nuodb/lib64/libNuoRemote.so
perl Makefile.PL
make test
sudo make install
```

## LICENSE ##

Copyright (c) 2012, NuoDB, Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

* Neither the name of NuoDB, Inc. nor the names of its contributors may
be used to endorse or promote products derived from this software
without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL NUODB, INC. BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

Related Pages:

[homepage]: http://www.nuodb.com
