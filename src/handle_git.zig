const std = @import("std");
const debug = std.debug.print;

const app = @import("app.zig");
const ManifestEntry = @import("manifest.zig").ManifestEntry;
const utils = app.utils;

const ArgList = []const []const u8;

// TODO: Move to some common area
fn exists(dir: std.fs.Dir, sub_path: []const u8) bool {
    _ = dir.statFile(sub_path) catch {
        var tmp_dir = dir.openDir(sub_path, .{}) catch {
            return false;
        };
        defer tmp_dir.close();
    };
    return true;
}

///! git update handler
pub fn update(allocator: std.mem.Allocator, entry: *ManifestEntry) !void {
    var src = entry.src.slice();
    var dst = entry.dst.slice();
    var cwd_scrap: [2048]u8 = undefined;
    const cwd = try std.fs.cwd().realpath(".", cwd_scrap[0..]);

    var scrap: [2048]u8 = undefined;
    var maybe_ref: ?[]const u8 = null;

    // Extract branch/ref-part
    if (std.mem.lastIndexOf(u8, src, "#")) |hash_idx| {
        maybe_ref = src[hash_idx + 1 ..];
        src = src[0..hash_idx];
    }

    // Dst does not exist: green field!
    if (!exists(std.fs.cwd(), dst)) {
        debug("DEBUG: Destination doesn't exists. Cloning.\n", .{});
        var args: ArgList = &.{ "git", "clone", src, dst };
        // TODO: Add silent-flag
        if ((try app.runCmdAndPrintAllPrefixed(allocator, args)) != 0) {
            debug("DEBUG: Got error executing git clone. See message(s) above.\n", .{});
        }
    } else {
        // Dst exists. Is it a repo (update it), or something else (fail)?
        var dst_repo_check = try std.fmt.bufPrint(scrap[0..], "{s}/.git", .{dst});
        if (exists(std.fs.cwd(), dst_repo_check)) {
            debug("DEBUG: Destination exists and seems like a repo. Let's pull.\n", .{});

            // Enter repo-dir, as git works on cwd
            try std.os.chdir(dst);
            defer std.os.chdir(cwd) catch unreachable;

            // Do git
            var args: ArgList = &.{ "git", "pull" };
            // TODO: Add silent-flag
            if ((try app.runCmdAndPrintAllPrefixed(allocator, args)) != 0) {
                debug("ERROR: Got error executing git pull. See message(s) above.", .{});
            }
        } else {
            debug("ERROR: Destination already exists, but is not a git-repo. Aborting.\n", .{});

            return error.DstExistsNotARepo;
        }
    }

    // Ref is specified: check it out
    if (maybe_ref) |ref| {
        debug("DEBUG: Checking out ref {s}\n", .{ref});

        // Enter repo-dor, as git works on cwd
        try std.os.chdir(dst);
        defer std.os.chdir(cwd) catch unreachable;

        // Do git
        var args: ArgList = &.{ "git", "checkout", ref };
        // TODO: Add silent-flag
        if ((try app.runCmdAndPrintAllPrefixed(allocator, args)) != 0) {
            debug("ERROR: Got error executing git checkout. See message(s) above.\n", .{});
        }
    }
}
