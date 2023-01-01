const std = @import("std");
const builtin = @import("builtin");
const testing = std.testing;
const debug = std.debug.print;

pub const utils = @import("utils.zig");
const argparse = @import("argparse.zig");
const Manifest = @import("manifest.zig").Manifest;
pub const ManifestEntry = @import("manifest.zig").ManifestEntry;
pub const EntryType = @import("manifest.zig").EntryType;
const Console = @import("console.zig").Console;
const handlers = @import("handlers/handlers.zig");
const io = @import("io.zig");

const SubCommand = argparse.SubCommand;

const AppErrors = error{ ArgumentError, ManifestError, ProcessError, UnknownError };

pub const APP_NAME = "prosit";
pub const APP_VERSION = blk: {
    if (builtin.mode != .Debug) {
        break :blk @embedFile("../VERSION");
    } else {
        break :blk @embedFile("../VERSION") ++ "-UNRELEASED";
    }
};

// TODO: add allocator here as well?
pub const Context = struct {
    console: Console,
    exec: io.CmdRunnerType,
    fn initDefault() Context {
        return .{
            .console = Console.initSimple(std.io.getStdOut().writer()),
            .exec = io.runCmdSilent,
        };
    }
};

///! Evalutes if subpath is inside dir (naively). Does not require subpath to actually exist.
fn subpathIsInDir(dir: []const u8, subpath: []const u8) bool {
    if (subpath.len < 2) return false;
    // TBD: This can be considered redundant now since we have the more specific check below, comparing resolved paths
    switch (subpath[0]) {
        '/', '\\' => return false, // Absolute paths
        '.' => {
            if (subpath[1] == '.') return false; // Navigate backwards
        },
        'a'...'z', 'A'...'Z' => {
            if (subpath[1] == ':') return false; // Windows drive
        },
        else => {}
    }

    // Attempt to resolve the subpath relative to dir
    var scrap_dir : [std.fs.MAX_PATH_BYTES]u8 = undefined;
    var scrap_subpath : [std.fs.MAX_PATH_BYTES]u8 = undefined;

    // Assumes dir actually exists
    var dir_path = std.fs.realpath(dir[0..], scrap_dir[0..]) catch return false;
    var subpath_fullpath = std.fmt.bufPrint(scrap_subpath[0..], "{s}/{s}", .{std.mem.trimRight(u8, dir_path, "\\/"), subpath}) catch return false; // TODO!
    
    // Weakly resolve subpath relative to dir
    std.mem.replaceScalar(u8, dir_path, '\\', '/'); // normalize
    std.mem.replaceScalar(u8, subpath_fullpath, '\\', '/'); // normalize
    
    while(std.mem.indexOf(u8, subpath_fullpath, "/..")) |idx| {
        // Replace until first / to the left
        if(std.mem.lastIndexOfScalar(u8, subpath_fullpath[0..idx], '/')) |sep_idx| {
            std.mem.copy(u8, subpath_fullpath[sep_idx..], subpath_fullpath[idx+3..]);
            subpath_fullpath = subpath_fullpath[0..subpath_fullpath.len-(idx-sep_idx+3)];
        } else {
            // We've ..'ed, but there's no preceeding dir
            return false;
        }
    }
    
    if(!std.mem.startsWith(u8, subpath_fullpath, dir_path)) return false;

    return true;
}


test "pathIsRelativeInside" {
    try testing.expect(subpathIsInDir("." , "./ok"));
    try testing.expect(subpathIsInDir("." , "ok"));
    try testing.expect(!subpathIsInDir(".", "/not/ok"));
    try testing.expect(!subpathIsInDir(".", "../not/ok"));
    try testing.expect(!subpathIsInDir(".", "c:\\not\\ok"));
    try testing.expect(!subpathIsInDir(".", "e:\\not\\ok"));
    try testing.expect(!subpathIsInDir(".", "\\not\\ok"));
    try testing.expect(!subpathIsInDir(".", "trying/../../to/fool/you"));
}

///! Does initial verification of high level correctness of manifest. E.g. with regards to out-of-tree-destination
fn precheckManifest(ctx: *Context, manifest: *Manifest, args: struct { require_in_ws: bool = true}) AppErrors!void {
    var all_ok: bool = true;

    for(manifest.entries.constSlice()) |entry| {
        if(!subpathIsInDir(".", entry.dst.constSlice())) {
            if(args.require_in_ws) {
                ctx.console.errorPrint("manifest line: {d}: destination may point to outside of project workspace ({s} and {s})\n", .{entry.source_line,".", entry.dst.constSlice()});
                all_ok = false;
            } else {
                ctx.console.debugPrint("manifest line: {d}: destination may point to outside of project workspace ({s} and {s})\n", .{entry.source_line,".", entry.dst.constSlice()});
            }
        }
    }

    if(!all_ok) {
        return AppErrors.ManifestError;
    }
}

///! Proper core entry point of tool
///! TODO: Consider wrapping tool as a struct if more context is needed
pub fn main(args: *argparse.AppArgs, envMap: *const std.BufMap) AppErrors!void {
    var aa = std.heap.ArenaAllocator.init(std.heap.page_allocator);
    defer aa.deinit();
    var allocator = aa.allocator();

    var stdout = std.io.getStdOut().writer();
    var stderr = std.io.getStdErr().writer();

    var context = Context{
        .console = Console.init(.{
            .std_writer = if (!args.silent) stdout else null,
            .debug_writer = if (args.verbose) stdout else null,
            .verbose_writer = if (args.verbose) stdout else null,
            .error_writer = if (!args.silent) stderr else null,
            .colors = .on,
        }),
        .exec = if (args.silent) io.runCmdSilent else io.runCmdAndPrintAllPrefixed,
    };

    var scrap: [2048]u8 = undefined;
    context.console.stdPrint("Workspace: {s}\n", .{std.fs.cwd().realpath(".", scrap[0..]) catch "UNKNOWN"});
    context.console.stdPrint("Manifest: {s}\n", .{args.manifest_path.slice()});

    // Parse manifest
    // Read file
    var manifest_data = io.readFile(allocator, std.fs.cwd(), args.manifest_path.slice()) catch |e| {
        context.console.errorPrint("Could not read manifest: {s}\n", .{@errorName(e)});
        return AppErrors.ManifestError;
    };
    var manifest = Manifest.fromBuf(manifest_data, envMap) catch |e| {
        context.console.errorPrint("Could not parse manifest: {s}\n", .{@errorName(e)});
        return AppErrors.ManifestError;
    };

    try precheckManifest(&context, &manifest, .{.require_in_ws = !args.allow_out_of_tree});

    // Handle according to subcommand
    if (args.subcommand) |subcommand| switch (subcommand) {
        SubCommand.update => {
            for (manifest.entries.slice()) |*entry| {
                context.console.stdPrint("Processing manifest entry @ line {d}: {s} {s} > {s}\n", .{ entry.source_line, @tagName(entry.entry_type), entry.src.constSlice(), entry.dst.constSlice() });
                handlers.update(allocator, &context, entry) catch {
                    context.console.errorPrint("Update failed for entry at line: {d}\n", .{entry.source_line});
                    return AppErrors.ProcessError;
                };
            }
        },
    } else {
        return AppErrors.ArgumentError;
    }
}

///! CLI entry point
///! TBD: Support multiple output modes? E.g. stdout/err vs write to buffer which can be analyzed?
pub fn cliMain(args: [][]const u8, envMap: *std.BufMap) AppErrors!void {
    var parsedArgs = argparse.parseArgs(args[0..]) catch |e| switch (e) {
        error.OkExit => {
            return;
        },
        error.MissingRequiredArguments => {
            debug("ERROR: No arguments provided\n", .{});
            argparse.printHelp(false);
            return AppErrors.ArgumentError;
        },
        else => {
            debug("ERROR: Got error parsing arguments ({s}). Aborting.", .{@errorName(e)});
            return AppErrors.ArgumentError;
        },
    };

    // Do, then convert common errors to appropraite error messages
    try main(&parsedArgs, envMap);
}
