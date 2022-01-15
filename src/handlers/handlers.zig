const std = @import("std");
const testing = std.testing;

const app = @import("../app.zig");

///! Interface definition for update-handlers
const HandlerFunc = fn (std.mem.Allocator, *app.Context, *app.ManifestEntry) anyerror!void;

///! Array index by enum representing each update-handler
const handlers = blk: {
    const fields_info = std.meta.fields(app.EntryType);

    var handlers_arr: [fields_info.len]HandlerFunc = undefined;

    handlers_arr[@enumToInt(app.EntryType.file)] = @import("handle_file.zig").update;
    handlers_arr[@enumToInt(app.EntryType.git)] = @import("handle_git.zig").update;

    break :blk handlers_arr;
};

///! Convenience-function forwarding a manifest-entry to appropriate handler
pub fn update(allocator: std.mem.Allocator, ctx: *app.Context, entry: *app.ManifestEntry) anyerror!void {
    return handlers[@enumToInt(entry.entry_type)](allocator, ctx, entry);
}
