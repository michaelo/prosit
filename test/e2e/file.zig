const std = @import("std");
const testing = std.testing;
const TestHelper = @import("helper.zig").TestHelper;

// test "traverse_parents - fails for windows" {
//     var scrap: [2048]u8 = undefined;

//     var cur = try std.fs.cwd().openDir(".", .{ .iterate=true});
//     debug("dir 1: {s}\n", .{try cur.realpath(".", scrap[0..])});
//     var parent = try cur.openDir("..", .{});
//     debug("dir 2: {s}\n", .{try parent.realpath(".", scrap[0..])});
//     var grandparent = try parent.openDir("..", .{});
//     debug("dir 3: {s}\n", .{try grandparent.realpath(".", scrap[0..])});
// }

test "e2e:file overall"
{
    var expected_entries = [_][]const u8{"dummy.txt", "folder/dummy_woop.txt", "folder2/dummy.txt"};
    try TestHelper.update(
        \\file: $(PROSIT_ITEST_TESTFILES)/dummy.txt > dummy.txt
        \\file: $(PROSIT_ITEST_TESTFILES)/dummy.txt > folder/dummy_woop.txt
        \\file: $(PROSIT_ITEST_TESTFILES)/dummy.txt > folder2/
        , expected_entries[0..]);
}

test "e2e:shall fail if src does not exist"
{
//     // Fails if src does not exist
//     ASSERT_NE(test_allinone(R"manifest(
// file: /no/such/file/can/possibly/exist > file.txt
//     )manifest"), App_Status_Code::Ok);
}
