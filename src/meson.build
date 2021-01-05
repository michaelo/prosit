src_lib_files = files([
    'app.cpp'
])

src_exe_files = files([
    'main.cpp',
])

deps = [
]

src_lib = library('app', src_lib_files, dependencies : deps)

exe = executable('prosit', src_exe_files,
    link_with : [src_lib],
    install : true
    )