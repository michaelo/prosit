const std = @import("std");
const testing = std.testing;
const TestHelper = @import("helper.zig").TestHelper;

test "e2e:file"
{
    var expected_entries = [_][]const u8{"dummy.txt", "folder/dummy_woop.txt", "folder2/dummy.txt"};
    try TestHelper.simpleUpdateAndAssert(
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
