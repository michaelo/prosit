// Test suite entry point. Import any file with e2e-tests to be run as part of it.
test "e2e:main" {
    _ = @import("file.zig");
    _ = @import("git.zig");
}