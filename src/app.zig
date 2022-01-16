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
    exec: fn (std.mem.Allocator, *Context, []const []const u8) io.ExecErrors!usize,
    fn initDefault() Context {
        return .{
            .console = Console.initSimple(std.io.getStdOut().writer()),
            .exec = io.runCmdSilent,
        };
    }
};

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
            .verbose_writer = if (!args.verbose) null else stdout,
            .error_writer = if (!args.silent) stderr else null,
            .colors = .on,
        }),
        .exec = if (args.silent) io.runCmdSilent else io.runCmdAndPrintAllPrefixed,
    };

    // Parse manifest
    // Read file
    var manifest_data = io.readFile(allocator, std.fs.cwd(), args.manifest_path.slice()) catch |e| {
        context.console.errorPrint("Could not read manifest: {s}\n", .{e});
        return AppErrors.ManifestError;
    };
    var manifest = Manifest.fromBuf(manifest_data, envMap) catch |e| {
        context.console.errorPrint("Could not parse manifest: {s}\n", .{e});
        return AppErrors.ManifestError;
    };

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
            debug("ERROR: Got error parsing arguments ({s}). Aborting.", .{e});
            return AppErrors.ArgumentError;
        },
    };

    // Do, then convert common errors to appropraite error messages
    try main(&parsedArgs, envMap);
}
