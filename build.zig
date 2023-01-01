const std = @import("std");

pub fn build(b: *std.build.Builder) void {
    // Standard target options allows the person running `zig build` to choose
    // what target to build for. Here we do not override the defaults, which
    // means any target is allowed, and the default is native. Other options
    // for restricting supported target set are available.
    const target = b.standardTargetOptions(.{});

    // Standard release options allow the person running `zig build` to select
    // between Debug, ReleaseSafe, ReleaseFast, and ReleaseSmall.
    const mode = b.standardReleaseOptions();

    const exe = b.addExecutable("prosit", "src/main.zig");
    exe.setMainPkgPath(".");
    exe.linkLibC(); // TODO: Evaluate if we can avoid
    exe.setTarget(target);
    exe.setBuildMode(mode);
    exe.install();

    const run_cmd = exe.run();
    run_cmd.step.dependOn(b.getInstallStep());
    if (b.args) |args| {
        run_cmd.addArgs(args);
    }

    const run_step = b.step("run", "Run the app");
    run_step.dependOn(&run_cmd.step);

    var exe_tests = b.addTest("src/app.zig");
    exe_tests.setBuildMode(mode);

    const test_step = b.step("test", "Run unit tests");
    test_step.dependOn(&exe_tests.step);

    if(target.isNative()) {
        // Create a separate itest-exe that links the main app?
        var exe_itests = b.addTest("test/e2e/main.zig");
        exe_itests.linkLibC();
        exe_itests.setTarget(target);
        exe_itests.setBuildMode(mode);
        exe_itests.setFilter("e2e:"); // Run only tests prefixed with "integration:"
        exe_itests.setMainPkgPath("."); // To allow access to src/ as well

        // exe_itests.linkSystemLibrary("c");
        // exe_itests.linkSystemLibrary("libcurl");

        const itest_step = b.step("itest", "Run default integration test suite");
        itest_step.dependOn(&exe_itests.step);
    }

}
