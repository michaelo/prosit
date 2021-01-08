project('prosit', 'cpp',
  version : '0.1.0',
  default_options : ['warning_level=3', 'cpp_std=c++17'])

subdir('src') # provides src_exe_files

subdir('test')

exe = executable('prosit', src_exe_files,
    link_with : [src_lib],
    install : true
    )