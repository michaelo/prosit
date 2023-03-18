const std = @import("std");
const app = @import("app.zig");

pub fn main() anyerror!void {
    var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);

    defer arena.deinit();
    const aa = arena.allocator();

    const args = try std.process.argsAlloc(aa);
    defer std.process.argsFree(aa, args);

    var envBufMap = blk: {
        var envMap = try std.process.getEnvMap(aa);
        defer envMap.deinit();

        var map = std.BufMap.init(aa);

        var envI = envMap.iterator();

        while (envI.next()) |env| {
            try map.put(env.key_ptr.*, env.value_ptr.*);
        }

        break :blk map;
    };
    defer envBufMap.deinit();

    app.cliMain(args[1..], &envBufMap) catch {
        std.process.exit(1);
    };
}
