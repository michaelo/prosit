project('prosit', 'cpp',
  version : '0.1.0',
  default_options : [
    'warning_level=3',
    meson.get_compiler('cpp').get_id() == 'msvc' ? 'cpp_std=vc++latest' : ''])

subdir('src') # provides src_lib

subdir('test/unit')
subdir('test/integration')

src_exe_files = files([
    'src/main.cpp',
])

exe = executable('prosit', src_exe_files,
    link_with : [src_lib],
    install : true
    )