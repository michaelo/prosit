# prosit

ATT! The WIP is strong with this one, but it's running now. Tested building for macOS (clang) and Windows 10 (msbuild).

The gist:

    prosit update # Update all dependencies as specifed in the manifest
    prosit init # Set up a new best-effort project.manifest in the current working directory
    prosit --help


## What it is

This is a minimalistic tool to help maintain your dependencies, with a high degree of flexibility with regards to their data source and how they are put in your workspace.

## The manifest-file 

A file in the current working directory (most likely the root-folder of your project). 

Name: project.manifest

Contents-syntax (Att! subject to changes):

    # Comments are supported. Here we pull a repo and checkout e.g. a tag
    git: https://github.com/some/repo#v1.0 > my/local/path
    # You can also specify branch or exact revision
    git: https://github.com/other/repo#main > otherrepo
    # Some files from somewhere else could be relevant
    file: /etc/hosts > local/hosts.txt
    # File from web
    https: http://michaelodden.com/robots.txt > robots.txt
    # File from web w/ basic auth and env-variables
    https: https://$(USERNAME):$(PASSWORD)@my.com/protected_file > protected_file

Assuming all sources are available, and any rights etc are OK, this shall result in a directory structure like this:

    ./project.manifest <- the file as described above
    ./my/local/path/
        .git/
        <checked out repo contents>
    ./otherrepo/
        .git/
        <checked out repo contents>
    ./local/
        hosts.txt

### Supported data sources

Supported for v1.0:
* git
* file
* https/http (with basic auth)

Future:
* dir
* hg
* https (with other auths)


## Build and install

It builds using the [Meson Build system](https://mesonbuild.com/).

    meson build
    cd build
    ninja
    ./prosit --help

### Prerequisites

* libcurl must be installed and available


## Design goals

* High performant (to the degree possible for an network/io-bound application)
* Get out of the way as fast as possible
* Flexible with regards to supported data sources and their in-workspace residency
* Support/encourage accuracy
* Zero runtime dependencies
* Minimal compile-time dependencies

## Target platforms

* x64 macOS (developed on)
* x64 Windows
* x64 Linux


## FAQ

Add it and they will come, right?


## License

See [LICENSE](LICENSE).