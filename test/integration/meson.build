testlib = [
    dependency('gtest', fallback:['gtest', 'gtest_dep']),
]

common_files = [
    'common.cpp',
]

tests_files = [
    'linkage_test.cpp',
    'main_test.cpp',
    'file_test.cpp',
    'git_test.cpp',
    'hg_test.cpp',
    'https_test.cpp',
]
foreach test_file: tests_files
    test(test_file,
        executable('test_prosit_' + test_file.underscorify(),
            [test_file, common_files],
            link_with: src_lib,
            include_directories: include_directories('../../src', '../'),
            dependencies: testlib,
            native: true),
        suite: 'integration')
endforeach

# Heavy tests - must be run deliberately
# TODO: Temporarily removed to avoid issues when testing setting up automatic builds
# test(test_file,
#     executable('test_prosit_stresstest',
#         ['stresstest.cpp', common_files],
#         link_with: src_lib,
#         include_directories: include_directories('../../src', '../'),
#         dependencies: testlib,
#         native: true),
#     suite: 'heavy',
#     timeout: 500)