const std = @import("std");
const os = @import("std").os;

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const json_dep = b.dependency("json", .{
        .target = target,
        .optimize = optimize,
    });

    const dbs_dep = b.dependency("dynamic_bitset", .{
        .target = target,
        .optimize = optimize,
    });

    const csv_dep = b.dependency("csv", .{
        .target = target,
        .optimize = optimize,
    });

    const phylourny_lib = b.addStaticLibrary(.{
        .name = "phylourny_lib",
        .target = target,
        .optimize = optimize,
    });
    phylourny_lib.linkLibC();
    phylourny_lib.linkLibCpp();
    phylourny_lib.addSystemIncludePath(dbs_dep.path("include"));
    phylourny_lib.addSystemIncludePath(csv_dep.path(""));
    phylourny_lib.addCSourceFiles(.{ .files = &.{
        "src/tournament.cpp",
        "src/model.cpp",
        "src/match.cpp",
        "src/util.cpp",
        "src/tournament_node.cpp",
        "src/tournament_factory.cpp",
        "src/single_node.cpp",
        "src/mcmc.cpp",
        "src/program_options.cpp",
        "src/results.cpp",
        "src/sampler.cpp",
        "src/cli.cpp",
    } });

    const phylourny = b.addExecutable(.{
        .name = "phylourny",
        .target = target,
        .optimize = optimize,
    });

    phylourny.linkLibC();
    phylourny.linkLibCpp();

    phylourny.addSystemIncludePath(json_dep.path("include/nlohmann"));
    phylourny.addSystemIncludePath(json_dep.path("include"));
    phylourny.addSystemIncludePath(dbs_dep.path("include"));
    phylourny.addSystemIncludePath(csv_dep.path(""));
    phylourny.linkLibrary(phylourny_lib);
    phylourny.addCSourceFiles(.{ .files = &.{
        "src/main.cpp",
    } });

    phylourny.root_module.addCMacro("GIT_REV", "v1.2.1-zig");

    b.installArtifact(phylourny);
}
