const std = @import("std");

const app = @import("../app.zig");
const ManifestEntry = app.ManifestEntry;

const ArgList = []const []const u8;
const HandlersErrors = @import("handlers.zig").HandlersErrors;
const fileOrFolderExists = @import("handlers.zig").fileOrFolderExists;

const httpclient = @import("../clients/httpclient.zig");

pub fn update(allocator: std.mem.Allocator, ctx: *app.Context, entry: *ManifestEntry) HandlersErrors!void {
    httpclient.init() catch {
        ctx.console.errorPrint("Could not initiate CURL\n", .{});
        return HandlersErrors.UnknownError;
    };
    defer httpclient.deinit();
    
    entry.src.buffer[entry.src.slice().len] = 0;
    var response = httpclient.request(allocator, .GET, entry.src.buffer[0..entry.src.slice().len:0], .{}) catch |e| {
        ctx.console.errorPrint("Fatal error requesting resource: {s}\n", .{@errorName(e)});
        return HandlersErrors.UnknownError;
    };
    defer response.deinit();

    if(response.response_type == .Error) {
        ctx.console.errorPrint("Got unexpected HTTP-response: {d}\n", .{response.http_code});
        return HandlersErrors.ExternalError;
    }

    // TODO: extract file/folder-extraction logics to helper - as is great overlap between handlers
    var dst = entry.dst.slice();

    if(response.body) |body| {
        ctx.console.debugPrint("Got data from remote: {s}\n", .{body.items});

        // Determine if we need to create subfolders for destination:
        var dst_folder_chunk: ?[]const u8 = null;
        var dst_file_chunk: ?[]const u8 = null;

        switch (dst[dst.len - 1]) {
            '/', '\\' => {
                ctx.console.errorPrint("Destination must be an explicit file\n", .{});
                return HandlersErrors.DstError;
            },
            else => {
                // dst is file, use this name
                if (std.mem.lastIndexOfAny(u8, dst, "\\/")) |sep_idx| {
                    dst_folder_chunk = dst[0..sep_idx];
                    dst_file_chunk = dst[sep_idx + 1 ..];
                } else {
                    dst_file_chunk = dst;
                }
            },
        }

        var dest_dir: std.fs.Dir = blk: {
            if(dst_folder_chunk) |dst_subpath| {
                break :blk std.fs.cwd().makeOpenPath(dst_subpath, .{}) catch |e| {
                    ctx.console.errorPrint("Could not create destination directory ({s})\n", .{@errorName(e)});
                    return HandlersErrors.DstError;
                };
            } else {
                break :blk std.fs.cwd();
            }
        };

        // Got data, write to file
        var file = dest_dir.createFile(dst_file_chunk.?, .{}) catch |e| {
            ctx.console.errorPrint("Could not create destination: {s} ({s})\n", .{entry.dst.slice(), @errorName(e)});
            return HandlersErrors.ExternalError;
        };
        defer file.close();

        file.writeAll(body.items) catch |e| {
            ctx.console.errorPrint("Could not write to destination: {s} ({s})", .{entry.dst.slice(), @errorName(e)});
        };
    }
}