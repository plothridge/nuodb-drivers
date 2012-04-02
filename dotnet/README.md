# NuoDB Database Provider for the Microsoft .NET Framework #

This is the .NET database provider for [NuoDB](http://www.nuodb.com).

## License ##

This software is provided under the BSD license:

* [NuoDB License](../LICENSE)

## Status ##

The NuoDB .NET Provider is currently _pre-release_ and is under active development, meaning it's pretty much a skeleton of what can be used in a production environment.  Please keep up with the [NuoDB Driver GitHub](https://github.com/nuodb/nuodb-drivers) project for updates.

## Requirements ##

* Microsoft Visual Studio 2010
* Microsoft .NET 3.5/4 Framework
* Windows version of NuoDB installed

## Building ##

Using VS2010, open the appropriate solution:

* **nuodb.sln** - for .NET 4
* **nuodb35.sln** - for .NET 3.5

The build presumes that you have installed the NuoDB API at `c:\opt\NuoDB\api`.

## Known Issues ##

There is a known issue using `msclr::interop::marshal_context` when building for .NET 3.5.
