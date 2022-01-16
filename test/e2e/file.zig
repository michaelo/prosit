const std = @import("std");
const testing = std.testing;
const TestHelper = @import("helper.zig").TestHelper;

test "e2e:file"
{
    var expected_entries = [_][]const u8{"dummy.txt", "dummy2.txt", "folder/dummy_woop.txt", "folder2/dummy.txt"};
    try TestHelper.simpleUpdateAndAssert(
        \\file: $(PROSIT_ITEST_TESTFILES)/dummy.txt > ./
        \\file: $(PROSIT_ITEST_TESTFILES)/dummy.txt > dummy2.txt
        \\file: $(PROSIT_ITEST_TESTFILES)/dummy.txt > folder/dummy_woop.txt
        \\file: $(PROSIT_ITEST_TESTFILES)/dummy.txt > folder2/
        , expected_entries[0..]);
}

test "e2e:shall fail if src does not exist"
{
    try testing.expectError(error.ProcessError, TestHelper.simpleUpdateAndAssert(
        \\file: /no/such/file/can/possibly/exist > file.txt
        , undefined));
}
