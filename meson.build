project('prosit', 'cpp',
  version : '1.0.0-rc1',
  default_options : [
    'warning_level=3',
    'cpp_std=c++17',
    ])
add_global_arguments('-DCURL_STATICLIB', language: 'cpp')

subdir('src') # provides src_lib

subdir('test/unit')
subdir('test/integration')

src_exe_files = files([
    'src/main.cpp',
])

exe = executable('prosit', src_exe_files,
    link_with : [src_lib],
    install : true,
    )