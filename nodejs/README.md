# db-nuodb: NuoDB database bindings for Node.js #

For detailed information about this and other Node.js
database bindings visit the [Node.js db-nuodb homepage] [homepage].

## BUILDING ##

```bash
git submodule update --init

export PATH=/usr/local/bin:$PATH
export NODE_PATH=/usr/local/bin/node
export NUODB_INCLUDE_DIR=/opt/nuodb/include
export NUODB_LIB_DIR=/opt/nuodb/lib64

node-waf configure && node-waf build
```

## INSTALL ##

Before proceeding with installation, you need to have NuoDB installed;
the examples below assume an installation location of /opt/nuodb, but
the location may vary.

In order for the installation script to locate dependencies properly, you'll 
need to set the NUODB_INCLUDE_DIR and NUODB_LIB_DIR environment variables. 
For example:

```bash
$ export NUODB_INCLUDE_DIR=/opt/nuodb/include
$ export NUODB_LIB_DIR=/opt/nuodb/lib64
```

Once the environment variables are set, install with npm:

```bash
$ npm install db-nuodb
```

Note: Until such time as we properly set ORIGIN/RPATH you may need to explicitly
set the LD_LIBRARY_PATH:

```bash
export LD_LIBRARY_PATH=/home/user-name/.node_libraries/.npm/db-nuodb/0.1.0/package/build/default/
```

Also, verify you have both these files installed to the same directory as the
nuo_bindings.node file:

  libNuoRemote.so

## QUICK CHECK ##

Run the following command to verify the shared library can be loaded by node:

```bash
$ node
````

In node run the following Javascript commands, then CTRL-C twice to exit:

```javascript
> var mod = require('./db-nuodb');
undefined
> mod;
{}
> 
(^C again to quit)
````

## QUICK START ##

```javascript
var nuodb = require('db-nuodb');
new nuodb.Database({
    hostname: 'localhost',
    user: 'root',
    password: 'password',
    database: 'node'
}).connect(function(error) {
    if (error) {
        return console.log("CONNECTION ERROR: " + error);
    }

    this.query().select('*').from('users').execute(function(error, rows) {
        if (error) {
            return console.log('ERROR: ' + error);
        }
        console.log(rows.length + ' ROWS');
    });
});
```

## LICENSE ##

This module is released under the [NUODB License] [license].

[homepage]: https://github.com/nuodb/nuodb-drivers/tree/master/nodejs
[license]: https://github.com/nuodb/nuodb-drivers/blob/master/LICENSE
