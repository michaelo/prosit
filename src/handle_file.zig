const std = @import("std");

const ManifestEntry = @import("manifest.zig").ManifestEntry;
const app = @import("app.zig");

const debug = std.debug.print;

///!
pub fn update(allocator: std.mem.Allocator, entry: *ManifestEntry) !void {
    _ = allocator;
    // If dst ends with dir-separator: keep source-name
    // If dst-folder doesn't exist: create it
    // TBD: If src is folder: copy all contents?
    // TBD: If dst-file exists, check timestamp? For now: always overwrite

    var src = entry.src.slice();
    var dst = entry.dst.slice();
    // debug("  src: {s}\n", .{src});
    // debug("  dst: {s}\n", .{dst});

    // TBD: use stat() or similar to proper evaluate wether anything is file, folder etc?
    var src_folder_chunk: ?[]const u8 = null;
    var src_file_chunk: ?[]const u8 = null;

    switch (src[src.len - 1]) {
        '/', '\\' => {
            // src is folder, currently not supported
            return error.SourceIsFolder;
        },
        else => {
            // src is file, we're a'good!

            if (std.mem.lastIndexOfAny(u8, src, "\\/")) |sep_idx| {
                src_folder_chunk = src[0..sep_idx];
                src_file_chunk = src[sep_idx + 1 ..];
            } else {
                src_file_chunk = src;
            }
        },
    }

    var dst_folder_chunk: ?[]const u8 = null;
    var dst_file_chunk: ?[]const u8 = null;

    switch (dst[dst.len - 1]) {
        '/', '\\' => {
            // dst is folder, keep name from src
            dst_folder_chunk = dst;
            dst_file_chunk = src_file_chunk;
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

    // debug("Copy from |{s}|, |{s}| to |{s}|, |{s}|\n", .{src_folder_chunk, src_file_chunk, dst_folder_chunk, dst_file_chunk});

    // Copy src to dst
    var dest_dir: std.fs.Dir = blk: {
        if(dst_folder_chunk) |dst_subpath| {
            break :blk try std.fs.cwd().makeOpenPath(dst_subpath, .{});
        } else {
            break :blk std.fs.cwd();
        }
    };

    try dest_dir.copyFile(src, dest_dir, dst_file_chunk.?, .{});
}
