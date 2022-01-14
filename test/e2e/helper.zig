const std = @import("std");
const testing = std.testing;
const app = @import("../../src/app.zig");
const utils = app.utils;
const debug = std.debug.print;

pub const TestHelper = struct {
    ///! Starts with the passed dir, and checks if it contains "needle". If not; check ancestor by ancestor until found. Or return null if never found.
    fn getFirstDirContaining(dir: std.fs.Dir, needle: []const u8) ?std.fs.Dir {
        var scrap: [2048]u8 = undefined;
        var path = dir.realpath(".", scrap[0..]) catch return null;

        while(true) {
            var checkFor = std.fmt.bufPrint(scrap[0..], "{s}{c}{s}", .{path, std.fs.path.sep, needle}) catch return null;
            if(std.fs.cwd().statFile(checkFor)) {
                // Att! Opens with the dir-seperator to allow opening root-dir on Windows
                return std.fs.cwd().openDir(checkFor[0..path.len+1], .{}) catch return null;
            } else |_| {
                if(std.mem.lastIndexOfAny(u8, path, "/\\")) |idx| {
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
        for(files) |file| {
            _ = dir.statFile(file) catch { return error.TestUnexpectedResult; };
        }
    }

    pub fn update(manifest_contents: []const u8, expected_structure: [][]const u8) !void {
        // Get path to testfiles-folder, and set it as env
        var testfiles_path_buf: [2048]u8 = undefined;
        
        var project_root_dir = getFirstDirContaining(std.fs.cwd(), "build.zig") orelse return error.NoRootDirFound;
        defer project_root_dir.close();

        var project_root_chunk = try project_root_dir.realpath(".", testfiles_path_buf[0..]);
        var testfiles_path = try std.fmt.bufPrint(testfiles_path_buf[0..], "{s}{s}", .{project_root_chunk, "/test/testfiles"});

        // Setup env
        var env = std.BufMap.init(std.testing.allocator);
        defer env.deinit();
        try env.put("PROSIT_ITEST_TESTFILES", testfiles_path);

        // Setup tmp-structure in which to populate based on manifest
        var tmp_ws_wdir = std.testing.tmpDir(.{});
        defer tmp_ws_wdir.cleanup(); // Doesn't seem to clean up the actual folder, only its contents (win?)

        // Set newly created tmp worskapce dir as cwd
        var prev_cwd = std.fs.cwd();
        defer utils.chdir(prev_cwd) catch unreachable;

        try utils.chdir(tmp_ws_wdir.dir);

        try createManifest(tmp_ws_wdir.dir, manifest_contents);

        // Process
        var args = [_][]const u8{"update"};
        try app.cliMain(args[0..], env);

        // Evaluate results
        try assertDirContains(tmp_ws_wdir.dir, expected_structure);
    }
};
