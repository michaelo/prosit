# Integration tests

## Strategy

Each integration test will set up a temp folder, set that as cwd, then call prosit with a specific manifest-file. After run, the results will be evaluated, and the folder cleaned.