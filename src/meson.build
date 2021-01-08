src_lib_files = files([
    'app.cpp',
    'manifest.cpp',
    'argparse.cpp',
    'handle_git.cpp',
    'handle_file.cpp',
    'handle_https.cpp',
    'platform.cpp',
])

deps = [
    # dependency('openssl'),
    dependency('libcurl')
]

conf_data = configuration_data()
conf_data.set('version', meson.project_version())
configure_file(input : 'prosit_config.h.in',
               output : 'prosit_config.h',
               configuration : conf_data)
configuration_inc = include_directories('.')

src_lib = static_library('app', src_lib_files, dependencies: deps, include_directories : configuration_inc)
