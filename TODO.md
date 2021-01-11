# TODO

## Now
* Consider if project.manifest should be prosit.manifest to reduce collision-risk
* Automatic builds to generate binaries for common platforms:
  * x64 Windows
  * x64 macOS
  * x64 Linux
* Investigate valgrind-warnings re: libcurl-usage ("Use of uninitialised value of size 8")
* Add argument to control if it shall quit at first error?

## Now or future
* Implement hg support
* Implement dir support
* Implement support for other data sources?
* Manifest: Support groups to allow only processing certain sets of entries. Syntax?
* More HTTP/S authentication methods:
    * Bearer?
    - Evaluate e.g. what Artifactory and such uses

## Future
* Recursive processing? I.e. if a pulled resource contains `project.manifest` - shall we process it?
  * Concerns:
    * Increases importance of ensuring our system()-calls are safe as we pass in strings from the manifest as parameters.
    * Should likely require case-by-case opt-in in manifest-file.
* Support user/global config file?
  * Allow fixed definitions for e.g. which git-executable to use?
* Define proper external API to allow utility to be used as a library?
* Manifest: Support annotations to entries, e.g:
  * !: same as -f, but for single entry
  * r: recursive, if dependency contains a project.manifest, then process it
  * ... 
