const std = @import("std");
const testing = std.testing;
const TestHelper = @import("helper.zig").TestHelper;

test "e2e:git shall clone" {
    var expected_entries = [_][]const u8{ "cloned/to/prosit/.git", "cloned/to/prosit/README.md" };
    try TestHelper.simpleUpdateAndAssert(
        \\git: https://github.com/michaelo/_prosit_itest > cloned/to/prosit
    , expected_entries[0..]);
}

test "e2e:git shall support change of branch/ref" {
    var context = try TestHelper.init();
    defer context.deinit();

    // Step 1
    {
        var expected_entries = [_][]const u8{ "cloned/to/prosit/.git", "cloned/to/prosit/README.md" };

        try context.updateAndAssert(
            \\git: https://github.com/michaelo/_prosit_itest#main > cloned/to/prosit
        , expected_entries[0..]);
    }

    // Step 2
    {
        var expected_entries = [_][]const u8{ "cloned/to/prosit/README_devbranch.md" };

        try context.updateAndAssert(
            \\git: https://github.com/michaelo/_prosit_itest#dev > cloned/to/prosit
        , expected_entries[0..]);
    }
}
