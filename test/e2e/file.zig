const std = @import("std");
const testing = std.testing;
const app = @import("../../src/app.zig");
const utils = app.utils;
const debug = std.debug.print;

fn assertDirContains(dir: std.fs.Dir, files: [][]const u8) !void {
    for(files) |file| {
        _ = dir.statFile(file) catch { return error.TestUnexpectedResult; };
    }
}

fn createManifest(dir: std.fs.Dir, manifest_contents: []const u8) !void {
    var file = try dir.createFile("prosit.manifest", .{});
    defer file.close();
    
    try file.writeAll(manifest_contents);
}

test "e2e:shall_copy_to_specific_file_name"
{
    // try app.do(undefined);
    var tmpDir = std.testing.tmpDir(.{});
    defer tmpDir.cleanup(); // Doesn't seem to clean up the actual folder, only its contents (win?)

    try utils.chdir(tmpDir.dir);

    // TODO: Set env PROSIT_ITEST_TESTFILES to ws/test/testfiles
    // TODO: Implement basic update handling for file - also: env-expansion when parsing manifest
    try createManifest(tmpDir.dir, 
        \\file: $(PROSIT_ITEST_TESTFILES)/dummy.txt > dummy.txt
        \\file: $(PROSIT_ITEST_TESTFILES)/dummy.txt > folder/dummy.txt
    );

    var args = [_][]const u8{"update"};
    try app.cliMain(args[0..]);

    var expected_entries = [_][]const u8{"dummy.txt", "folder/dummy.txt"};
    try assertDirContains(tmpDir.dir, expected_entries[0..]);
}

test "e2e:shall_copy_to_specific_folder_and_keep_source_name"
{
//     Test_Context context;
//     test_setup(&context);
//     defer(test_teardown(&context));

//     ASSERT_TRUE(test_run_with_manifest_and_contains_files(&context, R"(
// file: $(PROSIT_ITEST_TESTFILES)/dummy.txt > ./
// file: $(PROSIT_ITEST_TESTFILES)/dummy.txt > folder/
//     )", 2, 
//         TESTFILEARR{
//             "dummy.txt",
//             "folder/dummy.txt"
//         }));
}

test "e2e:shall_fail_if_src_doesnt_exist"
{
//     // Fails if src does not exist
//     ASSERT_NE(test_allinone(R"manifest(
// file: /no/such/file/can/possibly/exist > file.txt
//     )manifest"), App_Status_Code::Ok);
}
