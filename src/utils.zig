const std = @import("std");

pub fn chdir(dir: std.fs.Dir) !void {
    var scrap: [2048]u8 = undefined;
    var fullpath = try dir.realpath(".", scrap[0..]);
    try std.os.chdir(fullpath);
}
