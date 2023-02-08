const std = @import("std");

const app = @import("../app.zig");
const ManifestEntry = app.ManifestEntry;

const ArgList = []const []const u8;
const HandlersErrors = @import("handlers.zig").HandlersErrors;
const fileOrFolderExists = @import("handlers.zig").fileOrFolderExists;

const httpclient = @import("../clients/httpclient.zig");

pub fn update(allocator: std.mem.Allocator, ctx: *app.Context, entry: *ManifestEntry) HandlersErrors!void {
    _ = allocator;
    _ = ctx;
    _ = entry;
    
}