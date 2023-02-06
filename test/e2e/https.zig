const std = @import("std");
const testing = std.testing;
const TestHelper = @import("helper.zig").TestHelper;

// https://raw.githubusercontent.com/michaelo/_prosit_itest/main/README.md

test "e2e:https shall copy file from URL to local path" {
    var expected_entries = [_][]const u8{ "cloned/to/prosit/README.md" };
    try TestHelper.simpleUpdateAndAssert(
        \\https: https://raw.githubusercontent.com/michaelo/_prosit_itest/main/README.md > cloned/to/prosit/
    , expected_entries[0..]);
}