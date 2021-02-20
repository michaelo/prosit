src_lib_files = files([
    'argparse.cpp',
    'app.cpp',
    'handle_file.cpp',
    'handle_git.cpp',
    'handle_hg.cpp',
    'handle_https.cpp',
    'manifest.cpp',
    'platform.cpp',
    'utils.cpp',
])

deps = [
    # dependency('openssl'),
    dependency('libcurl', static:true)
]

conf_data = configuration_data()
conf_data.set('version', meson.project_version())
configure_file(input : 'prosit_config.h.in',
               output : 'prosit_config.h',
               configuration : conf_data)
configuration_inc = include_directories('.')

src_lib = static_library('app',
                src_lib_files,
                dependencies: deps,
                include_directories : configuration_inc,
                override_options : [meson.get_compiler('cpp').get_id() == 'msvc' ? 'cpp_std=vc++latest' : 'cpp_std=c++17'])
