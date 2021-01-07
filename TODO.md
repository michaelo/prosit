# TODO

## Now
* Settle on format and name for manifest-file
* Automatic builds to generate binaries for common platforms:
  * x64 Windows
  * x64 macOS
  * x64 Linux
* Implement git support
* Implement file support
* Implement HTTP/S support w/ basic auth
* Manifest: error handling for invalid definitions


## Now or future
* Implement hg support
* Implement dir support
* Implement support for other data sources?
* Manifest: Support groups
* HTTP/S authentication methods:
    * Basic auth
    * Bearer?
    * NTLM?
    - Evaluate e.g. what Artifactory and such uses

## Future
* Recursive processing? I.e. if a pulled resource contains `project.manifest` - shall we process it?
  * Concerns:
    * Increases importance of ensuring our system()-calls are safe as we pass in strings from the manifest as parameters.
    * Should likely require case-by-case opt-in in manifest-file.
* Support user/global config file?
  * Allow fixed definitions for e.g. which git-executable to use?
* Define proper external API to allow utility to be used as a library?