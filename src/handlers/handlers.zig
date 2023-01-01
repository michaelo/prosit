const std = @import("std");
const testing = std.testing;

const app = @import("../app.zig");

pub const HandlersErrors = error {
    SrcError,
    DstError,
    ExternalError,
    UnknownError,
    OutOfBounds,
};

///! Interface definition for update-handlers
const HandlerFunc = *const fn (std.mem.Allocator, *app.Context, *app.ManifestEntry) HandlersErrors!void;

///! Array index by enum representing each update-handler
const handlers = blk: {
    const fields_info = std.meta.fields(app.EntryType);

    var handlers_arr: [fields_info.len]HandlerFunc = undefined;

    handlers_arr[@enumToInt(app.EntryType.file)] = @import("handle_file.zig").update;
    handlers_arr[@enumToInt(app.EntryType.git)] = @import("handle_git.zig").update;

    break :blk handlers_arr;
};

///! Convenience-function forwarding a manifest-entry to appropriate handler
pub fn update(allocator: std.mem.Allocator, ctx: *app.Context, entry: *app.ManifestEntry) HandlersErrors!void {
    return handlers[@enumToInt(entry.entry_type)](allocator, ctx, entry);
}

test "smoke test" {
    _ = handlers;
}

pub fn fileOrFolderExists(dir: std.fs.Dir, sub_path: []const u8) bool {
    _ = dir.statFile(sub_path) catch {
        var tmp_dir = dir.openDir(sub_path, .{}) catch {
            return false;
        };
        defer tmp_dir.close();
    };
    return true;
}