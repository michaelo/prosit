const std = @import("std");
const testing = std.testing;
const TestHelper = @import("helper.zig").TestHelper;

// https://raw.githubusercontent.com/michaelo/_prosit_itest/main/README.md

test "e2e:https shall copy file from URL to local path" {
    var expected_entries = [_][]const u8{ "cloned/to/prosit/README.md", "othername" };
    try TestHelper.simpleUpdateAndAssert(
        \\https: https://raw.githubusercontent.com/michaelo/_prosit_itest/main/README.md > cloned/to/prosit/README.md
        \\https: https://raw.githubusercontent.com/michaelo/_prosit_itest/main/README.md > othername
    , expected_entries[0..]);
}

test "e2e:https shall support basic auth" {
    var expected_entries = [_][]const u8{ "file.txt" };
    try testing.expectError(error.ProcessError, TestHelper.simpleUpdateAndAssert(
        \\https: https://src.michaelodden.com/prosit/basicauth/file.txt > file.txt
    , expected_entries[0..]));

    try testing.expectError(error.ProcessError, TestHelper.simpleUpdateAndAssert(
        \\https: https://testuser:incorrectpass@src.michaelodden.com/prosit/basicauth/file.txt > file.txt
    , expected_entries[0..]));


    try TestHelper.simpleUpdateAndAssert(
        \\https: https://testuser:testpass@src.michaelodden.com/prosit/basicauth/file.txt > file.txt
    , expected_entries[0..]);
}