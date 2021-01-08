src_lib_files = files([
    'app.cpp',
    'manifest.cpp',
    'argparse.cpp',
    'handle_git.cpp',
    'handle_file.cpp',
    'handle_https.cpp',
    'platform.cpp',
])

src_exe_files = files([
    'main.cpp',
])

deps = [
    # dependency('openssl'),
    dependency('libcurl')
]

src_lib = static_library('app', src_lib_files, dependencies: deps)
