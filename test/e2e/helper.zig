const std = @import("std");
const testing = std.testing;
const app = @import("../../src/app.zig");
const utils = app.utils;
const debug = std.debug.print;

pub const TestHelper = struct {
    const Self = @This();

    ///! Starts with the passed dir, and checks if it contains "needle". If not; check ancestor by ancestor until found. Or return null if never found.
    ///! ATT! recursively calling .openDir("..") fails on Windows, so then we resort to this strategy.
    fn getFirstDirContaining(dir: std.fs.Dir, needle: []const u8) ?std.fs.Dir {
        var scrap: [2048]u8 = undefined;
        var path = dir.realpath(".", scrap[0..]) catch return null;

        while (true) {
            var checkFor = std.fmt.bufPrint(scrap[0..], "{s}{c}{s}", .{ path, std.fs.path.sep, needle }) catch return null;
            if (std.fs.cwd().statFile(checkFor)) {
                // Att! Opens with the dir-seperator to allow opening root-dir on Windows
                return std.fs.cwd().openDir(checkFor[0 .. path.len + 1], .{}) catch return null;
            } else |_| {
                if (std.mem.lastIndexOfAny(u8, path, "/\\")) |idx| {
                    path = path[0..idx];
                } else {
                    return null;
                }
            }
        }
        return null;
    }

    fn createManifest(dir: std.fs.Dir, manifest_contents: []const u8) !void {
        var file = try dir.createFile("prosit.manifest", .{});
        defer file.close();

        try file.writeAll(manifest_contents);
    }

    fn assertDirContains(dir: std.fs.Dir, files: [][]const u8) !void {
        for (files) |file| {
            _ = dir.statFile(file) catch |e| {
                var tmp_dir = dir.openDir(file, .{}) catch |e2| {
                    debug("TEST ERROR: Expected to find '{s}', but checking it as file resulted in '{s}', and as dir in '{s}'\n", .{file, e, e2});
                    unreachable;
                };
                defer tmp_dir.close();
            };
        }
    }

    ///! Convenience-function to create a workspace with given manifest-contents, perform update on it, and evaluate contents
    pub fn simpleUpdateAndAssert(manifest_contents: []const u8, expected_structure: [][]const u8) !void {
        var context = try Self.init();
        defer context.deinit();

        try context.updateAndAssert(manifest_contents, expected_structure);
    }

    tmp_dir: std.testing.TmpDir,
    original_cwd: std.BoundedArray(u8, 2048),
    env: std.BufMap,

    pub fn init() !Self {
        var testfiles_path_buf: [2048]u8 = undefined;

        var project_root_dir = getFirstDirContaining(std.fs.cwd(), "build.zig") orelse return error.NoRootDirFound;
        defer project_root_dir.close();

        var project_root_chunk = try project_root_dir.realpath(".", testfiles_path_buf[0..]);
        var testfiles_path = try std.fmt.bufPrint(testfiles_path_buf[0..], "{s}{s}", .{ project_root_chunk, "/test/testfiles" });

        // Setup env
        var env = std.BufMap.init(std.testing.allocator); // Must be deinit()'d
        try env.put("PROSIT_ITEST_TESTFILES", testfiles_path);

        // Setup tmp-structure in which to populate based on manifest

        // Set newly created tmp worskapce dir as cwd
        var scrap: [2048]u8 = undefined;
        var orirignal_cwd = try std.fs.cwd().realpath(".", scrap[0..]);

        var tmp_ws_wdir = std.testing.tmpDir(.{}); // Must ble .cleanup()'d
        try utils.chdir(tmp_ws_wdir.dir);

        return Self{ .tmp_dir = tmp_ws_wdir, .env = env, .original_cwd = try std.BoundedArray(u8, 2048).fromSlice(orirignal_cwd) };
    }

    pub fn updateAndAssert(self: *Self, maybe_manifest_contents: ?[]const u8, expected_structure: [][]const u8) !void {
        // Define manifest
        if (maybe_manifest_contents) |manifest_contents| {
            try createManifest(self.tmp_dir.dir, manifest_contents);
        }

        // Process
        var args = [_][]const u8{"update"};
        try app.cliMain(args[0..], &self.env);

        // Evaluate results
        try assertDirContains(self.tmp_dir.dir, expected_structure);
    }

    pub fn deinit(self: *Self) void {
        std.os.chdir(self.original_cwd.slice()) catch unreachable;
        self.env.deinit();
        self.tmp_dir.cleanup();
    }
};

// test "traverse_parents - fails for windows" {
//     var scrap: [2048]u8 = undefined;

//     var cur = try std.fs.cwd().openDir(".", .{ .iterate=true});
//     debug("dir 1: {s}\n", .{try cur.realpath(".", scrap[0..])});
//     var parent = try cur.openDir("..", .{});
//     debug("dir 2: {s}\n", .{try parent.realpath(".", scrap[0..])});
//     var grandparent = try parent.openDir("..", .{});
//     debug("dir 3: {s}\n", .{try grandparent.realpath(".", scrap[0..])});
// }
