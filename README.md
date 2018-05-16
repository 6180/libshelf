# shelf
shelf is an attempt at a simple alternative to a few select tools in the ERESI Reverse Engineering Software Interface. It is designed to provide a scriptable or interactive interface for manipulating ELF binaries. It is mostly intended to ease the pain of static and runtime modification of ELF binaries (executables, shared objects, core dumps and relocatable objects). 

Some planned features include:

* Injection of shared libraries and relocatable object code.
* Several forms of function hooking/redirection including plt injection, DT_DEBUG to DT_NEEDED conversion, function trampolining and .ctor/.dtor overwriting.
* Provide a shell like interface for inspecting and modifying ELF binaries. Will give full read/write access to any data structures in the ELF file.
* Static and dynamic ELF file relinking.
* Automation of common ELF infection techniques like Silvio .text padding infection.
* Provide alternatives to GNU readelf and objdump for stripped binaries with partial section table rebuilding.