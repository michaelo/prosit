const std = @import("std");
const builtin = @import("builtin");

const testing = std.testing;
const debug = std.debug.print;

pub const utils = @import("utils.zig");

pub const APP_NAME = "prosit";
pub const APP_VERSION = blk: {
    if (builtin.mode != .Debug) {
        break :blk @embedFile("../VERSION");
    } else {
        break :blk @embedFile("../VERSION") ++ "-UNRELEASED";
    }
};

const errors = error{ ParseError, CouldNotReadManifest, ProcessError };

const SubCommand = enum {
    update,
    fn fromString(str: []const u8) !SubCommand {
        return std.meta.stringToEnum(SubCommand, str) orelse return error.InvalidSubCommand;
    }
};

test "SubCommand" {
    try testing.expectEqual(SubCommand.update, try SubCommand.fromString("update"));
    try testing.expectError(error.InvalidSubCommand, SubCommand.fromString("blah"));
}

const AppArgs = struct {
    subcommand: ?SubCommand = null,
    manifest_path: std.BoundedArray(u8, 1024) = std.BoundedArray(u8, 1024).fromSlice("manifest.json") catch unreachable,
    force: bool = false,
    silent: bool = false,
    verbose: bool = false,
    allow_out_of_tree: bool = false,
};

pub fn printHelp(full: bool) void {
    debug(
        \\{0s} v{1s} - Simple dependency retrieval tool
        \\
        \\Usage: {0s} --help|<subcommand> [arguments]
        \\
    , .{ APP_NAME, APP_VERSION });

    if (!full) {
        debug(
            \\
            \\try '{0s} --help' for more information.
            \\
        , .{APP_NAME});
        return;
    }

    debug(
        \\
        \\Subcommands:
        \\    update  Attempt to ensure the workspace is in sync with the manifest
        \\ 
        \\Examples:
        \\    {0s} update
        \\
        \\Arguments:
        \\    -h, --help     This help
        \\    -f, --force    Override in case of destructive actions
        \\        --manifest Override default manifest name/path (default: prosit.manifest)
        \\    -x, --outoftree     Required for manifests specifying destinations outside of the
        \\                        directory of the manifest
        \\    -s, --silent   Suppress all output except error messages
        \\    -v, --verbose  Show debug output
        \\        --version  Show version and quit
        \\
        \\(c) Michael Odden - https://github.com/michaelo/prosit
        \\
    , .{APP_NAME});
}

fn argIs(arg: []const u8, full: []const u8, short: ?[]const u8) bool {
    return std.mem.eql(u8, arg, full) or std.mem.eql(u8, arg, short orelse "321NOSUCHTHING123");
}

fn argHasValue(arg: []const u8, full: []const u8, short: ?[]const u8) ?[]const u8 {
    var eq_pos = std.mem.indexOf(u8, arg, "=") orelse return null;

    var key = arg[0..eq_pos];

    if (argIs(key, full, short)) {
        return arg[eq_pos + 1 ..];
    } else return null;
}

fn parseArgs(args: [][]const u8) !AppArgs {
    if (args.len < 1) {
        debug("ERROR: No arguments provided\n", .{});
        printHelp(false);
        return error.NoArguments;
    }

    var result = AppArgs{};

    // Strategy (for later):
    // Scan args for valid subcommand, any args before: global args, any args after: subcommand-specific?

    for (args[0..]) |arg| {
        // Flags
        if (argIs(arg, "--help", "-h")) {
            printHelp(true);
            return error.OkExit;
        }

        if (argIs(arg, "--version", null)) {
            debug("{0s} v{1s}\n", .{ APP_NAME, APP_VERSION });
            return error.OkExit;
        }

        if (argIs(arg, "--verbose", "-v")) {
            result.verbose = true;
            continue;
        }

        if (argIs(arg, "--silent", "-s")) {
            result.silent = true;
            continue;
        }

        if (argIs(arg, "--force", "-f")) {
            result.force = true;
            continue;
        }

        if (argIs(arg, "--outoftree", "-x")) {
            result.allow_out_of_tree = true;
            continue;
        }

        if (arg[0] == '-') {
            debug("ERROR: Unsupported argument '{s}'\n", .{arg});
            printHelp(false);
            return error.InvalidArgument;
        }

        // Check for anything that looks like a subcommand
        if (result.subcommand == null) {
            if (SubCommand.fromString(arg)) |subcommand| {
                result.subcommand = subcommand;
                continue;
            } else |_| {
                // Do nothing...
            }
        }

        debug("WARNING: Unhandled argument: '{s}'\n", .{arg});
    }

    return result;
}

test "parseArgs" {
    {
        var args = [_][]const u8{"update"};
        var parsed = try parseArgs(args[0..]);
        try testing.expectEqual(AppArgs{
            .subcommand = SubCommand.update,
        }, parsed);
    }

    {
        var args = [_][]const u8{"update", "--verbose"};
        var parsed = try parseArgs(args[0..]);
        try testing.expectEqual(AppArgs{
            .subcommand = SubCommand.update,
            .verbose = true
        }, parsed);
    }

    {
        var args = [_][]const u8{"update", "--outoftree"};
        var parsed = try parseArgs(args[0..]);
        try testing.expectEqual(AppArgs{
            .subcommand = SubCommand.update,
            .allow_out_of_tree = true
        }, parsed);
    }
}

///! Proper core entry point of tool
pub fn main(args: *AppArgs) errors!void {
    if (args.subcommand) |subcommand| switch (subcommand) {
        SubCommand.update => {
            // TODO: Implement update-logic
        },
    } else {
        debug("ERROR: No subcommand provided.\n", .{});
    }
}

///! CLI-wrapper
pub fn cliMain(args: [][]const u8) anyerror!void {
    var parsedArgs = parseArgs(args[0..]) catch |e| switch(e) {
        error.OkExit => { return; },
        else => { return e; }
    };

    // Do, then convert common errors to appropraite error messages
    return main(&parsedArgs) catch |e| switch (e) {
        errors.CouldNotReadManifest => {
            debug("Could not read manifest-file: {s}\n", .{parsedArgs.manifest_path.slice()});
        },
        else => {
            debug("DEBUG: Unhandled error: {s}\n", .{e});
        },
    };
}
