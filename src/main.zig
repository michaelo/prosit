const std = @import("std");
const app = @import("app.zig");

pub fn main() anyerror!void {
    var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);

    defer arena.deinit();
    const aa = arena.allocator();

    const args = try std.process.argsAlloc(aa);
    defer std.process.argsFree(aa, args);

    var envMap = try std.process.getEnvMap(aa);
    defer envMap.deinit();

    app.cliMain(args[1..], envMap) catch {
        std.process.exit(1);
    };
}
