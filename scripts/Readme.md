# Scripts

Contains the build scripts and some debug helper files (natvis and natstepfilter)

My dev env is Windows 11 with clang, msvc, and cygwin.

By default I use clang, however I tested the builds with all three vendors using `build_all_vendors`

The *ci* version of the scripts are intended to run in an automated environment such as github actikons.
The only difference is I insert a cls in the ones I use on my machine.
