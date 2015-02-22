# Coder

Coder is a source code editor. Besides general editing functionality Coder will provide C++ intellisence.

Coder aims to be a resource efficient tool, both memory and CPU wise, for editing and reviewing C++ source code.

Coder parses the C++ document together with all #include files when possible to provide assistance when reading or editing the C++ file:

* Go to declaration, definition and references of symbols and identifiers.
* Context sensitive suggestions while editing.
* Support navigation throughout the file.
* Hilighting of read/write access of variables.
* Tools for refactoring, like renaming.

Support for other languages gets added eventually.

# Current Features

The current feature set includes keyboard based editing without support of CTRL-Z/CTRL-Y.
Coder provides syntax hilighting for C++ and hilighting of search results.

# Planned Features

* Parsing C++ files and providing declaration, definition and references of symbols and identifiers.
* Hilighting parser errors.
* Providing context sensitive intellisence.
* Editing features for mouse input.
* Multi Document support.

# Build Coder

To build Coder Visual Studio 2013 is required. Open the solution at Projects/Win32/Coder.sln and build the Coder project.
