testlib = [
    dependency('gtest', fallback:['gtest', 'gtest_dep']),
]
tests_files = [
    'linkage_test.cpp',
    'main_test.cpp'
]
foreach test_file: tests_files
    test(test_file,
        executable('test_prosit_' + test_file.underscorify(),
            test_file,
            link_with: src_lib,
            include_directories: include_directories('../../src', '../'),
            dependencies: testlib,
            native: true),
        suite: 'integration')
endforeach