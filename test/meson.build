testlib = [
    dependency('gtest', fallback:['gtest', 'gtest_dep']),
    # dependency('cmocka', fallback:['cmocka', 'cmocka_dep'])
]
tests_files = [
    'test.cpp',
    'linkage_test.cpp',
    'manifest_test.cpp',
    'argparse_test.cpp'
]
foreach test_file: tests_files
    test(test_file,
        executable('test_prosit_' + test_file.underscorify(),
            test_file,
            link_with: src_lib,
            include_directories: include_directories('../src'),
            dependencies: testlib,
            native: true))
endforeach