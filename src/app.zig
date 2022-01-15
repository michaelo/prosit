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

const SubCommand = argparse.SubCommand;

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
    exec: fn (std.mem.Allocator, *Context, []const []const u8) ExecErrors!usize,
    fn initDefault() Context {
        return .{
            .console = Console.initSimple(std.io.getStdOut().writer()),
            .exec = runCmdSilent,
        };
    }
};

const errors = error{ ArgumentError, ManifestError, ProcessError, UnknownError };

///! If no error, user owns returned buf
pub fn readFile(allocator: std.mem.Allocator, dir: std.fs.Dir, path: []const u8) ![]u8 {
    var file = try dir.openFile(path, .{ .read = true });
    defer file.close();

    return try file.readToEndAlloc(allocator, 10 * 1024 * 1024);
}

///! Proper core entry point of tool
///! TODO: Consider wrapping tool as a struct if more context is needed
pub fn main(args: *argparse.AppArgs, envMap: *const std.BufMap) errors!void {
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
        .exec = if (args.silent) runCmdSilent else runCmdAndPrintAllPrefixed,
    };

    // Parse manifest
    // Read file
    var manifest_data = readFile(allocator, std.fs.cwd(), args.manifest_path.slice()) catch |e| {
        context.console.errorPrint("Could not read manifest: {s}\n", .{e});
        return errors.ManifestError;
    };
    var manifest = Manifest.fromBuf(manifest_data, envMap) catch |e| {
        context.console.errorPrint("Could not parse manifest: {s}\n", .{e});
        return errors.ManifestError;
    };

    // Handle according to subcommand
    if (args.subcommand) |subcommand| switch (subcommand) {
        SubCommand.update => {
            for (manifest.entries.slice()) |*entry| {
                context.console.stdPrint("Processing manifest entry @ line {d}: {s} {s} > {s}\n", .{ entry.source_line, @tagName(entry.entry_type), entry.src.constSlice(), entry.dst.constSlice() });
                handlers.update(allocator, &context, entry) catch {
                    context.console.errorPrint("Update failed for entry at line: {d}\n", .{entry.source_line});
                    return errors.ProcessError;
                };
            }
        },
    } else {
        return errors.ArgumentError;
    }
}

///! CLI entry point
///! TBD: Support multiple output modes? E.g. stdout/err vs write to buffer which can be analyzed?
pub fn cliMain(args: [][]const u8, envMap: *std.BufMap) errors!void {
    var parsedArgs = argparse.parseArgs(args[0..]) catch |e| switch (e) {
        error.OkExit => {
            return;
        },
        else => {
            debug("ERROR: Got error parsing arguments ({s}). Aborting.", .{e});
            return errors.ArgumentError;
        },
    };

    // Do, then convert common errors to appropraite error messages
    try main(&parsedArgs, envMap);
}

///! Autosense buffer for type of line ending: Check buf for \r\n, and if found: return \r\n, otherwise \n
fn getLineEnding(buf: []const u8) []const u8 {
    if (std.mem.indexOf(u8, buf, "\r\n") != null) return "\r\n";
    return "\n";
}

///! Splits buffer on autosensed line ending. Writes line-by-line prefix+line-contents to the passed writer()
fn printAllLinesWithPrefix(buf: []const u8, prefix: []const u8, writer: std.fs.File.Writer) void {
    if (std.mem.trim(u8, buf, " \t\n").len == 0) return;

    var line_end = getLineEnding(buf);
    var line_it = std.mem.split(u8, buf, line_end);
    while (line_it.next()) |line| {
        writer.print("{s}{s}{s}", .{ prefix, line, line_end }) catch {};
    }
}


const ExecErrors = error {
    ExecError
};
///! Chatty cmd-runner. Outputs verbatim what it does and what it gets
pub fn runCmdAndPrintAllPrefixed(allocator: std.mem.Allocator, ctx: *Context, cmd: []const []const u8) ExecErrors!usize {
    var scrap: [2048]u8 = undefined;
    ctx.console.stdPrint("Executing {s} (cwd: {s})\n", .{ cmd, std.fs.cwd().realpath(".", scrap[0..]) });

    var result = std.ChildProcess.exec(.{
        .allocator = allocator,
        .argv = cmd[0..],
        .env_map = null,
    }) catch |e| switch(e) {
        else => {
            return ExecErrors.ExecError;
        }
    };
    defer allocator.free(result.stderr);
    defer allocator.free(result.stdout);

    printAllLinesWithPrefix(result.stdout, "stdout: ", std.io.getStdOut().writer());
    printAllLinesWithPrefix(result.stderr, "stderr: ", std.io.getStdErr().writer());

    return result.term.Exited;
}

///! Silent cmd-runner. Outputs nothing.
pub fn runCmdSilent(allocator: std.mem.Allocator, ctx: *Context, cmd: []const []const u8) ExecErrors!usize {
    _ = ctx;

    var result = std.ChildProcess.exec(.{
        .allocator = allocator,
        .argv = cmd[0..],
        .env_map = null,
    }) catch |e| switch(e) {
        else => {
            return ExecErrors.ExecError;
        }
    };
    defer allocator.free(result.stderr);
    defer allocator.free(result.stdout);

    return result.term.Exited;
}

///! Utility function, returns a slice of references to the default values in the provided struct/tuple
///! TBD: Might not be useful unless we can support runtime-values as well
pub fn fields(cmd: anytype) [][]const u8 {
    const ArgsType = @TypeOf(cmd);
    if (@typeInfo(ArgsType) != .Struct) {
        @compileError("Expected tuple or struct argument, found " ++ @typeName(ArgsType));
    }
    const fields_info = std.meta.fields(ArgsType);

    comptime {
        var results: [fields_info.len][]const u8 = undefined;
        for (fields_info) |field_info, i| {
            results[i] = @field(cmd, field_info.name);
        }

        return results[0..];
    }
}

test "runCmd" {
    // var arg = "--help";
    _ = try runCmdAndPrintAllPrefixed(std.testing.allocator, &Context.initDefault(), fields(.{ "git", "--help" }));
}
