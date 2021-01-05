# prosit

The gist:

    prosit init . # Set up a new prosit-manifest in the current working directory
    prosit update # Update all dependencies as speicifed in the manifest
    prosit --help


## What it is

This is a minimalistic tool to help maintain your dependencies, with a high degree of flexibility with regards to their data source and how they are put in your workspace.

## The manifest-file

A file in the current working directory (most likely the root-folder of your project). 

Name: manifest.conf


## Build and install

It builds using the [Meson Build system](https://mesonbuild.com/).

    meson build
    cd build
    ninja
    ./src/prosit --help


## Design goals

* High performant
* Getting out of the way as fast as possible
* Flexible with regards to supported data sources and their in-workspace residency
* No runtime dependencies
* Minimal compile-time dependencies

## Tested platforms

* TBD


## FAQ

Add it and they will come, right?


## License

See [LICENSE](LICENSE).