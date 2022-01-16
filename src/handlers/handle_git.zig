const std = @import("std");

const app = @import("../app.zig");
const ManifestEntry = app.ManifestEntry;

const ArgList = []const []const u8;
const HandlersErrors = @import("handlers.zig").HandlersErrors;
const fileOrFolderExists = @import("handlers.zig").fileOrFolderExists;

///! git update handler
pub fn update(allocator: std.mem.Allocator, ctx: *app.Context, entry: *ManifestEntry) HandlersErrors!void {
    var src = entry.src.slice();
    var dst = entry.dst.slice();
    var cwd_scrap: [2048]u8 = undefined;
    const cwd = std.fs.cwd().realpath(".", cwd_scrap[0..]) catch |e| {
        ctx.console.errorPrint("Could not resolve cwd ({s})\n", .{e});
        return HandlersErrors.UnknownError;
    };

    var scrap: [2048]u8 = undefined;
    var maybe_ref: ?[]const u8 = null;

    // Extract branch/ref-part
    if (std.mem.lastIndexOf(u8, src, "#")) |hash_idx| {
        maybe_ref = src[hash_idx + 1 ..];
        src = src[0..hash_idx];
    }

    // Dst does not exist: green field!
    if (!fileOrFolderExists(std.fs.cwd(), dst)) {
        ctx.console.debugPrint("Destination doesn't exists. Cloning.\n", .{});
        var args: ArgList = &.{ "git", "clone", src, dst };
        if ((ctx.exec(allocator, ctx, args) catch {
            ctx.console.errorPrint("Unknown error when calling git\n", .{});
            return HandlersErrors.UnknownError;
        }) != 0) {
            ctx.console.debugPrint("Got error executing git clone. See message(s) above.\n", .{});
        }
    } else {
        // Dst exists. Is it a repo (update it), or something else (fail)?
        var dst_repo_check = std.fmt.bufPrint(scrap[0..], "{s}/.git", .{dst}) catch {
            return HandlersErrors.OutOfBounds;
        };
        if (fileOrFolderExists(std.fs.cwd(), dst_repo_check)) {
            ctx.console.debugPrint("Destination exists and seems like a repo. Let's pull.\n", .{});

            // Enter repo-dir, as git works on cwd
            std.os.chdir(dst) catch |e| {
                ctx.console.errorPrint("Could not change cwd to '{s}'' ({s})\n", .{ dst, e });
                return HandlersErrors.UnknownError;
            };
            defer std.os.chdir(cwd) catch unreachable;

            // Do git
            var args: ArgList = &.{ "git", "pull" };
            if ((ctx.exec(allocator, ctx, args) catch {
                ctx.console.errorPrint("Unknown error when calling git\n", .{});
                return HandlersErrors.UnknownError;
            }) != 0) {
                ctx.console.errorPrint("Got error executing git pull. See message(s) above.", .{});
            }
        } else {
            ctx.console.errorPrint("Destination already exists, but is not a git-repo. Aborting.\n", .{});

            return error.DstError;
        }
    }

    // Ref is specified: check it out
    if (maybe_ref) |ref| {
        ctx.console.debugPrint("Checking out ref {s}\n", .{ref});

        // Enter repo-dor, as git works on cwd
        std.os.chdir(dst) catch |e| {
            ctx.console.errorPrint("Could not change cwd to '{s}'' ({s})\n", .{ dst, e });
            return HandlersErrors.UnknownError;
        };
        defer std.os.chdir(cwd) catch unreachable;

        // Do git
        var args: ArgList = &.{ "git", "checkout", ref };
        if ((ctx.exec(allocator, ctx, args) catch {
            ctx.console.errorPrint("Unknown error when calling git\n", .{});
            return HandlersErrors.UnknownError;
        }) != 0) {
            ctx.console.errorPrint("Got error executing git checkout. See message(s) above.\n", .{});
        }
    }
}
