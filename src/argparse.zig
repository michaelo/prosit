const std = @import("std");
const builtin = @import("builtin");
const testing = std.testing;
const debug = std.debug.print;

const app = @import("app.zig");

pub const AppArgs = struct {
    subcommand: ?SubCommand = null,
    manifest_path: std.BoundedArray(u8, 1024) = std.BoundedArray(u8, 1024).fromSlice("prosit.manifest") catch unreachable,
    force: bool = false,
    silent: bool = false,
    verbose: bool = false,
    allow_out_of_tree: bool = false,
};

pub const SubCommand = enum {
    update,
    fn fromString(str: []const u8) !SubCommand {
        return std.meta.stringToEnum(SubCommand, str) orelse return error.InvalidSubCommand;
    }
};

test "SubCommand" {
    try testing.expectEqual(SubCommand.update, try SubCommand.fromString("update"));
    try testing.expectError(error.InvalidSubCommand, SubCommand.fromString("blah"));
}


pub fn printHelp(full: bool) void {
    debug(
        \\{0s} v{1s} - Simple dependency retrieval tool
        \\
        \\Usage: {0s} --help|<subcommand> [arguments]
        \\
    , .{ app.APP_NAME, app.APP_VERSION });

    if (!full) {
        debug(
            \\
            \\try '{0s} --help' for more information.
            \\
        , .{app.APP_NAME});
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
    , .{app.APP_NAME});
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

pub fn parseArgs(args: [][]const u8) !AppArgs {
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
            debug("{0s} v{1s}\n", .{ app.APP_NAME, app.APP_VERSION });
            return error.OkExit;
        }

        if (argHasValue(arg, "--manifest", null)) |arg_value| {
            result.manifest_path.resize(0) catch unreachable;
            try result.manifest_path.appendSlice(arg_value);
            continue;
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
        var args = [_][]const u8{ "update", "--verbose" };
        var parsed = try parseArgs(args[0..]);
        try testing.expectEqual(AppArgs{ .subcommand = SubCommand.update, .verbose = true }, parsed);
    }

    {
        var args = [_][]const u8{ "update", "--outoftree" };
        var parsed = try parseArgs(args[0..]);
        try testing.expectEqual(AppArgs{ .subcommand = SubCommand.update, .allow_out_of_tree = true }, parsed);
    }
}
