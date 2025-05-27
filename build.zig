const std = @import("std");

var gpa = std.heap.GeneralPurposeAllocator(.{}){};

const allocator = gpa.allocator();

pub fn build(b: *std.Build) !void {
    const release = b.option(bool, "release", "Disables debug symbols and enables all compiler optimization") orelse false;
    const target = b.standardTargetOptions(.{.default_target = .{ .abi = .gnu }});
    var c_flags = std.ArrayList([]const u8).init(allocator);
    try c_flags.append("-std=gnu99");
    try c_flags.append("-Wall");
    try c_flags.append("-Werror");
    try c_flags.append("-Wextra");
    if (release) {
        try c_flags.append("-O3");
    } else {
        try c_flags.append("-g");
    }
    var cpp_flags = std.ArrayList([]const u8).init(allocator);
    if (release) {
        try cpp_flags.append("-O3");
    } else {
        try cpp_flags.append("-g");
    }

    const exe = b.addExecutable(.{ 
        .name = "game",
        .target = target});


    var c_dir = try std.fs.cwd().openDir("src", .{ .iterate = true });
    defer c_dir.close();

    var dirIterator = c_dir.iterate();
    while (try dirIterator.next()) |dirContent| {
        exe.addCSourceFile(.{
            .file = b.path((try std.fmt.allocPrint(allocator, "src/{s}", .{dirContent.name})) ),
            .flags = c_flags.items,
        });
    }
    
    var cpp_dir = try std.fs.cwd().openDir("cpp", .{ .iterate = true });
    defer cpp_dir.close();

    dirIterator = cpp_dir.iterate();
    while (try dirIterator.next()) |dirContent| {
        exe.addCSourceFile(.{
            .file = b.path((try std.fmt.allocPrint(allocator, "cpp/{s}", .{dirContent.name})) ),
            .flags = cpp_flags.items,
        });
    }
    exe.addSystemIncludePath(b.path("include"));
    exe.addLibraryPath(b.path("lib"));

    if (target.result.os.tag == .linux) {
        exe.linkSystemLibrary("raylib");
    }
    if (target.result.os.tag == .windows) {
        exe.linkSystemLibrary("raylibdll");
    }
    
    if (target.result.os.tag == .windows) {
        exe.linkSystemLibrary("ws2_32");
    }


    exe.linkLibC();
    exe.linkLibCpp();
    
    b.installArtifact(exe);
}
