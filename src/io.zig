const std = @import("std");
const Console = @import("console.zig").Console;

///! Autosense buffer for type of line ending: Check buf for \r\n, and if found: return \r\n, otherwise \n
pub fn getLineEnding(buf: []const u8) []const u8 {
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

pub const ExecErrors = error {
    ExecError
};

pub const CmdRunnerType = *const fn (allocator: std.mem.Allocator, console: *Console, cmd: []const []const u8) ExecErrors!usize;

///! Chatty cmd-runner. Outputs verbatim what it does and what it gets
pub fn runCmdAndPrintAllPrefixed(allocator: std.mem.Allocator, console: *Console, cmd: []const []const u8) ExecErrors!usize {
    var scrap: [2048]u8 = undefined;
    console.stdPrint("Executing {s} (cwd: {s})\n", .{ cmd, std.fs.cwd().realpath(".", scrap[0..]) catch "UNKNOWN" });

    var result = std.ChildProcess.exec(.{
        .allocator = allocator,
        .argv = cmd[0..],
        .env_map = null,
    }) catch |e| {
        console.errorPrint("Got error executing command ({s})\n", .{@errorName(e)});
        return ExecErrors.ExecError;
    };
    defer allocator.free(result.stderr);
    defer allocator.free(result.stdout);

    printAllLinesWithPrefix(result.stdout, "stdout: ", std.io.getStdOut().writer());
    printAllLinesWithPrefix(result.stderr, "stderr: ", std.io.getStdErr().writer());

    return result.term.Exited;
}

///! Silent cmd-runner. Outputs nothing.
pub fn runCmdSilent(allocator: std.mem.Allocator, console: *Console, cmd: []const []const u8) ExecErrors!usize {
    _ = console;

    var result = std.ChildProcess.exec(.{
        .allocator = allocator,
        .argv = cmd[0..],
        .env_map = null,
    }) catch {
        return ExecErrors.ExecError;
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
    _ = try runCmdAndPrintAllPrefixed(std.testing.allocator, &Console.initSimple(std.io.getStdOut().writer()), fields(.{ "git", "--help" }));
}

///! If no error, user owns returned buf
pub fn readFile(allocator: std.mem.Allocator, dir: std.fs.Dir, path: []const u8) ![]u8 {
    var file = try dir.openFile(path, .{ .mode = .read_only });
    defer file.close();

    return try file.readToEndAlloc(allocator, 10 * 1024 * 1024);
}

