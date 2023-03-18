const std = @import("std");

// Although this function looks imperative, note that its job is to
// declaratively construct a build graph that will be executed by an external
// runner.
pub fn build(b: *std.Build) void {
    // Standard target options allows the person running `zig build` to choose
    // what target to build for. Here we do not override the defaults, which
    // means any target is allowed, and the default is native. Other options
    // for restricting supported target set are available.
    const target = b.standardTargetOptions(.{});

    // Standard optimization options allow the person running `zig build` to select
    // between Debug, ReleaseSafe, ReleaseFast, and ReleaseSmall. Here we do not
    // set a preferred release mode, allowing the user to decide how to optimize.
    const optimize = b.standardOptimizeOption(.{});

    const exe = b.addExecutable(.{
        .name = "prosit",
        // In this case the main source file is merely a path, however, in more
        // complicated build scripts, this could be a generated file.
        .root_source_file = .{ .path = "src/main.zig" },
        .target = target,
        .optimize = optimize,
    });
    exe.linkSystemLibrary("libcurl");

    exe.setMainPkgPath(".");

    // This declares intent for the executable to be installed into the
    // standard location when the user invokes the "install" step (the default
    // step when running `zig build`).
    exe.install();

    // This *creates* a RunStep in the build graph, to be executed when another
    // step is evaluated that depends on it. The next line below will establish
    // such a dependency.
    const run_cmd = exe.run();

    // By making the run step depend on the install step, it will be run from the
    // installation directory rather than directly from within the cache directory.
    // This is not necessary, however, if the application depends on other installed
    // files, this ensures they will be present and in the expected location.
    run_cmd.step.dependOn(b.getInstallStep());

    // This allows the user to pass arguments to the application in the build
    // command itself, like this: `zig build run -- arg1 arg2 etc`
    if (b.args) |args| {
        run_cmd.addArgs(args);
    }

    // This creates a build step. It will be visible in the `zig build --help` menu,
    // and can be selected like this: `zig build run`
    // This will evaluate the `run` step rather than the default, which is "install".
    const run_step = b.step("run", "Run the app");
    run_step.dependOn(&run_cmd.step);

    // Creates a step for unit testing.
    const exe_tests = b.addTest(.{
        .root_source_file = .{ .path = "src/app.zig" },
        .target = target,
        .optimize = optimize,
    });

    // Similar to creating the run step earlier, this exposes a `test` step to
    // the `zig build --help` menu, providing a way for the user to request
    // running the unit tests.
    const test_step = b.step("test", "Run unit tests");
    test_step.dependOn(&exe_tests.step);

    if(target.isNative()) {
        // Create a separate itest-exe that links the main app?
        var exe_itests = b.addTest(.{
            .root_source_file = .{ .path = "test/e2e/main.zig" },
            .target = target,
            .optimize = optimize,
        });
        exe_itests.setMainPkgPath("."); // To allow access to src/ as well
        
        exe_itests.setFilter("e2e:"); // Run only tests prefixed with "integration:"

        exe_itests.linkLibC();
        exe_itests.linkSystemLibrary("libcurl");

        const itest_step = b.step("itest", "Run default integration test suite");
        itest_step.dependOn(&exe_itests.step);

        // crossBuild(b, optimize) catch |e| {
        //     std.debug.print("Got error while crossbuilding: {s}\n", .{@errorName(e)});
        // };

    }
}


// Not working until we resolve/vendor platform-specific referencedy by curl-includes
fn crossBuild(b: *std.Build, optimize: std.builtin.Mode) !void {
    // Create job to generate crossbuilds for all desired targets
    // .target = try std.zig.CrossTarget.parse(.{ .arch_os_abi = "wasm32-freestanding" }),
    var prevStep: ?*std.build.Step = null;
    inline for([_][]const u8{ "x86_64-windows-gnu", "x86_64-macos", "x86_64-linux-gnu" }) |xbuild_target| {
        const tmpexe = b.addExecutable(.{
            .name = "prosit",
            // In this case the main source file is merely a path, however, in more
            // complicated build scripts, this could be a generated file.
            .root_source_file = .{ .path = "src/main.zig" },
            .target = try std.zig.CrossTarget.parse(.{ .arch_os_abi = xbuild_target }),
            .optimize = optimize,
        });
        tmpexe.addIncludePath("vendored/includes");
        try tmpexe.lib_paths.insert(0, std.Build.FileSource.relative("vendored/libs/" ++ xbuild_target));
        // tmpexe.addLibraryPath("vendored/libs/" ++ xbuild_target);
        tmpexe.linkSystemLibrary("libcurl");

        tmpexe.setMainPkgPath(".");
        tmpexe.output_dir = "xbuild/" ++ xbuild_target;

        // This declares intent for the executable to be installed into the
        // standard location when the user invokes the "install" step (the default
        // step when running `zig build`).
        tmpexe.install();

        if(prevStep) |prev| {
            tmpexe.step.dependOn(prev);
        }

        prevStep = &tmpexe.step;
    }

    if(prevStep) |prev| {
        const xbuild_step = b.step("crossbuild", "Crossbuild for a set of predefined targets");
        xbuild_step.dependOn(prev);
    }
}
