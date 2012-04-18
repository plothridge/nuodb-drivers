# NuoDB Database Provider for the Microsoft .NET Framework #

This is the .NET database provider for [NuoDB](http://www.nuodb.com).

## License ##

This software is provided under the BSD license:

* [NuoDB License](LICENSE)

## Status ##

The NuoDB .NET Provider is currently _pre-release_ and is under active development and not officially ready for a production environment.  Please keep up with the [NuoDB Driver GitHub](https://github.com/nuodb/nuodb-drivers) project for any updates.

## Implementation ##

This .NET Provider directly uses the NuoRemote client library shipped with NuoDB rather than the [Native API](https://github.com/nuodb/nuodb-api).  As C++/CLI must be used to consume either library, this is to reduce dependencies and to better provide as direct an interface to NuoDB as possible.

## Requirements ##

* Microsoft Visual Studio 2010
* Microsoft .NET 3.5 or 4.0 Framework
* Windows version of NuoDB installed at `c:\opt\NuoDB\`

## Building ##

Using VS2010, open the appropriate solution:

* **nuodb.sln** - for .NET 4
* **nuodb35.sln** - for .NET 3.5
