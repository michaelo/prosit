# TODO

## Features
### Now
* Automatic builds to generate binaries for common platforms:
  * x64 Windows
  * x64 macOS
  * x64 Linux
* Investigate valgrind-warnings re: libcurl-usage ("Use of uninitialised value of size 8")
* Add argument to control if it shall quit at first error?

### Now or future
* Extraction of .zip, .tar and .tar.gz (Solve as entry annotation?)
* Implement hg support
* Implement dir support
* Implement support for other data sources?
* Manifest: Support groups to allow only processing certain sets of entries. Syntax?
* More HTTP/S authentication methods:
    * Bearer?
    - Evaluate e.g. what Artifactory and such uses
* Sign .exe

### Future
* Recursive processing? I.e. if a pulled resource contains `prosit.manifest` - shall we process it?
  * Concerns:
    * Increases importance of ensuring our system()-calls are safe as we pass in strings from the manifest as parameters.
    * Should likely require case-by-case opt-in in manifest-file.
* Support user/global config file?
  * Allow fixed definitions for e.g. which git-executable to use?
* Define proper external API to allow utility to be used as a library?
* Manifest: Support annotations to entries, e.g:
  * !: same as -f, but for single entry
  * r: recursive, if dependency contains a prosit.manifest, then process it
  * x: extract if archive
  * ... 

## General / techdebt
* Clean up use of std. Make more consistent how and when it's used. Right now there are a lot of mix between char*, std::string and std::filesyste::path.
* Output-handling when --multithreaded (for system() in particular): Redirect to temporary files , then batch up and dump data once entry finished. Must lock output.
* test_teardown fails to delete temporary area after git-related tests

## Packaging

We want to provide self-contained executables that can easily drop in a predefined set of supported systems/platforms.

To achieve this we must statically link any dependencies.