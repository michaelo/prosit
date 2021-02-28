Notes regarding building on different platforms
===============================================

General prerequisites
---------------------

It builds using the [Meson Build system](https://mesonbuild.com/).

Linux/macOS (using Ninja):
---------------------

    meson build
    cd build
    ninja
    ./prosit --help

Windows (using Ninja)
---------------------
Note: Will investigate strategy for simpler builds on Windows, but this will do for now:

Prerequisites:

* Visual Studio 2019 (tested with)

Launch "x64 Native Tools Command Prompt for VS 2019" (i.e. a shell with the vcvars64.bat file pre set)

    meson build
    cd build
    ninja
    prosit --help
