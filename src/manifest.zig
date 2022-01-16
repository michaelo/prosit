const std = @import("std");
const builtin = @import("builtin");
const testing = std.testing;

const debug = std.debug.print;

const ManifestErrors = error{ InvalidSyntax, OutOfBounds, VariableNotFound, UnknownType };

pub const EntryType = enum {
    git,
    file,
    hg,
    https,

    fn fromString(str: []const u8) !EntryType {
        return std.meta.stringToEnum(EntryType, str) orelse return error.InvalidEntryType;
    }
};

pub const ManifestEntry = struct {
    source_line: usize,
    entry_type: EntryType,
    src: std.BoundedArray(u8, 2048),
    dst: std.BoundedArray(u8, 2048),

    fn create(source_line: usize, entry_type: []const u8, src: []const u8, dst: []const u8) ManifestErrors!ManifestEntry {
        return ManifestEntry{
            .source_line = source_line,
            .entry_type = EntryType.fromString(entry_type) catch {
                return ManifestErrors.UnknownType;
            },
            .src = std.BoundedArray(u8, 2048).fromSlice(src) catch {
                return ManifestErrors.OutOfBounds;
            },
            .dst = std.BoundedArray(u8, 2048).fromSlice(dst) catch {
                return ManifestErrors.OutOfBounds;
            },
        };
    }
};

// TODO: Can make fromBuf a standalone function which simply returns a BoundedArray.
pub const Manifest = struct {
    const Self = @This();
    entries: std.BoundedArray(ManifestEntry, 128) = std.BoundedArray(ManifestEntry, 128).init(0) catch unreachable,

    pub fn len(self: *Self) usize {
        return self.entries.constSlice().len;
    }

    pub fn slice(self: *Self) []ManifestEntry {
        return self.entries.slice();
    }

    pub fn constSlice(self: *Self) []ManifestEntry {
        return self.entries.constSlice();
    }

    pub fn precheck() bool {}

    fn parseLine(line_idx: usize, line: []const u8) ManifestErrors!ManifestEntry {
        // format: <type>: <source> > <dest>[flags]
        const type_sep = ": ";
        const src_dest_sep = " > ";

        if (std.mem.indexOf(u8, line, type_sep)) |idx_colon| {
            if (std.mem.indexOf(u8, line, src_dest_sep)) |idx_redirect| {
                var maybe_type = std.mem.trim(u8, line[0..idx_colon], " \t");
                var maybe_src = std.mem.trim(u8, line[idx_colon + type_sep.len .. idx_redirect], " \t");
                var maybe_dst = std.mem.trim(u8, line[idx_redirect + src_dest_sep.len ..], " \t");
                // TODO: Support flags

                return try ManifestEntry.create(line_idx, maybe_type, maybe_src, maybe_dst);
            }
        }

        return error.InvalidSyntax;
    }

    pub fn fromBuf(buf: []const u8, maybe_env: ?*const std.BufMap) ManifestErrors!Manifest {
        var result = Manifest{};

        var line_it = std.mem.tokenize(u8, buf, getLineEnding(buf));
        var line_idx: usize = 1;
        while (line_it.next()) |line_raw| : (line_idx += 1) {
            var line = std.mem.trim(u8, line_raw, " \t");

            // Skip empties and comments
            if (line.len == 0) continue;
            if (line[0] == '#') continue;

            // Parse entry and add to list
            result.entries.append(parseLine(line_idx, line) catch |e| {
                debug("ERROR: Failed parsing manifest.json line {d}: {s}\n", .{ line_idx, e });
                return e;
            }) catch {
                debug("ERROR: Reached max number of entries in manifest ({d}). File bug to project to make this dynamic.\n", .{result.entries.len});
                return ManifestErrors.OutOfBounds;
            };
        }

        // Expand variables
        if (maybe_env) |env| {
            for (result.entries.slice()) |*entry| {
                expandVariablesInBounded(entry.src.buffer.len, &entry.src, env) catch |e| {
                    debug("ERROR: Got error ({s}) expanding variables in line {d}\n", .{ e, entry.source_line });
                    return e;
                };
                expandVariablesInBounded(entry.dst.buffer.len, &entry.dst, env) catch |e| {
                    debug("ERROR: Got error ({s}) expanding variables in line {d}\n", .{ e, entry.source_line });
                    return e;
                };
            }
        }

        return result;
    }
};

///! Scans the str for $(var)-patterns, and if found attempts to look 'var' up from env and if found there; replace it in str.
fn expandVariablesInBounded(comptime capacity: usize, str: *std.BoundedArray(u8, capacity), env: *const std.BufMap) ManifestErrors!void {
    var start_idx: usize = 0;

    while (std.mem.indexOf(u8, str.slice()[start_idx..], "$(")) |cand_start| {
        if (std.mem.indexOf(u8, str.slice()[start_idx + cand_start ..], ")")) |cand_end| {
            // Got pair!
            var var_key = str.slice()[start_idx + cand_start + 2 .. start_idx + cand_start + cand_end];

            if (env.get(var_key)) |var_value| {
                str.replaceRange(start_idx + cand_start, cand_end + 1, var_value) catch {
                    return ManifestErrors.OutOfBounds;
                };
            }

            start_idx += cand_start + cand_end + 1;

            // Variable ends at end of string?
            if (start_idx >= str.slice().len) break;
        }
    }
}

test "expandVariablesInBounded" {
    var env = std.BufMap.init(std.testing.allocator);
    defer env.deinit();

    try env.put("var", "value");

    {
        var mystr = try std.BoundedArray(u8, 1024).fromSlice("$(var)");
        try expandVariablesInBounded(mystr.buffer.len, &mystr, &env);
        try testing.expectEqualStrings("value", mystr.slice());
    }

    {
        var mystr = try std.BoundedArray(u8, 1024).fromSlice("pre$(var)post");
        try expandVariablesInBounded(mystr.buffer.len, &mystr, &env);
        try testing.expectEqualStrings("prevaluepost", mystr.slice());
    }

    {
        var mystr = try std.BoundedArray(u8, 1024).fromSlice("pre$(var)in$(var)post");
        try expandVariablesInBounded(mystr.buffer.len, &mystr, &env);
        try testing.expectEqualStrings("prevalueinvaluepost", mystr.slice());
    }
}

test "Manifest.fromBuf" {
    // Empty
    {
        var buf =
            \\
        ;

        var manifest = try Manifest.fromBuf(buf, null);
        try testing.expectEqual(@as(usize, 0), manifest.len());
    }

    // Single entry, w comments
    {
        var buf =
            \\# git-repo from local path
            \\git: /my/remote/repo.git#main > ./path/to/local/clone
        ;
        var manifest = try Manifest.fromBuf(buf, null);
        try testing.expectEqual(@as(usize, 1), manifest.len());
        try testing.expectEqual(EntryType.git, manifest.slice()[0].entry_type);
    }

    // Multi entry, w comments and blank lines
    {
        var buf =
            \\# Comment here
            \\file: /my/file.txt > ./local/path/to/file.txt
            \\
            \\# Other comment here
            \\git: https://localhost:8999/dummy.git#tagv1 > ./path/to/local/clone
        ;
        var manifest = try Manifest.fromBuf(buf, null);
        try testing.expectEqual(@as(usize, 2), manifest.len());

        var entry_one = manifest.slice()[0];
        try testing.expectEqual(EntryType.file, entry_one.entry_type);
        try testing.expectEqualStrings("/my/file.txt", entry_one.src.slice());
        try testing.expectEqualStrings("./local/path/to/file.txt", entry_one.dst.slice());

        var entry_two = manifest.slice()[1];
        try testing.expectEqual(EntryType.git, entry_two.entry_type);
        try testing.expectEqualStrings("https://localhost:8999/dummy.git#tagv1", entry_two.src.slice());
        try testing.expectEqualStrings("./path/to/local/clone", entry_two.dst.slice());
    }
}

test "Manifest.fromBuf shall expand environment variables" {
    // Single entry, w comments
    {
        var buf =
            \\git: $(src_repo)#$(src_ref) > $(dst_path)
        ;
        var env = std.BufMap.init(std.testing.allocator);
        defer env.deinit();

        try env.put("src_repo", "somerepouri");
        try env.put("src_ref", "main");
        try env.put("dst_path", "./local/path/here");

        var manifest = try Manifest.fromBuf(buf, &env);
        try testing.expectEqual(@as(usize, 1), manifest.len());
        try testing.expectEqual(EntryType.git, manifest.slice()[0].entry_type);
        try testing.expectEqualStrings("somerepouri#main", manifest.slice()[0].src.slice());
        try testing.expectEqualStrings("./local/path/here", manifest.slice()[0].dst.slice());
    }
}

pub fn getLineEnding(buf: []const u8) []const u8 {
    if (std.mem.indexOf(u8, buf, "\r\n") != null) return "\r\n";
    return "\n";
}
