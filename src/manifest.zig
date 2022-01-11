const std = @import("std");
const builtin = @import("builtin");
const testing = std.testing;

const debug = std.debug.print;

// TODO: Determine strategy. Pass in a slice with capacity, and return a new length?
// fn expandEnvVars()

const EntryType = enum {
    git,
    file,
    hg,
    https,

    fn fromString(str: []const u8) !EntryType {
        return std.meta.stringToEnum(EntryType, str) orelse return error.InvalidEntryType;
    }
};

const ManifestEntry = struct {
    source_line: usize,
    entry_type: EntryType,
    src: std.BoundedArray(u8, 2048),
    dst: std.BoundedArray(u8, 2048),

    fn create(source_line: usize, entry_type: []const u8, src: []const u8, dst: []const u8) !ManifestEntry {
        return ManifestEntry{
            .source_line = source_line,
            .entry_type = try EntryType.fromString(entry_type),
            .src = try std.BoundedArray(u8, 2048).fromSlice(src),
            .dst = try std.BoundedArray(u8, 2048).fromSlice(dst),
        };
    }
};

// TODO: Can make fromBuf a standalone function which simply returns a BoundedArray.
const Manifest = struct {
    const Self = @This();
    entries: std.BoundedArray(ManifestEntry, 128) = std.BoundedArray(ManifestEntry, 128).init(0) catch unreachable,

    fn len(self: *Self) usize {
        return self.entries.constSlice().len;
    }

    fn slice(self: *Self) []ManifestEntry {
        return self.entries.slice();
    }

    fn constSlice(self: *Self) []ManifestEntry {
        return self.entries.constSlice();
    }

    fn precheck() bool {}

    
    fn fromBuf(buf: []const u8) !Manifest {
        // TBD: allocate?
        var result = Manifest{};

        var line_it = std.mem.tokenize(u8, buf, getLineEnding(buf));
        var line_idx: usize = 0;
        while(line_it.next()) |line_raw| : (line_idx += 1) {
            var line = std.mem.trim(u8, line_raw, " \t");

            // Skip empties and comments
            if(line.len == 0) continue;
            if(line[0] == '#') continue;

            // Parse entry
            // format: <type>: <source> > <dest>[flags]
            const type_sep = ": ";
            const src_dest_sep = " > ";

            if(std.mem.indexOf(u8, line, type_sep)) |idx_colon| {
                if(std.mem.indexOf(u8, line, src_dest_sep)) |idx_redirect| {
                    var maybe_type = std.mem.trim(u8, line[0..idx_colon], " \t");
                    var maybe_src = std.mem.trim(u8, line[idx_colon+type_sep.len..idx_redirect], " \t");
                    var maybe_dst = std.mem.trim(u8, line[idx_redirect+src_dest_sep.len..], " \t");
                    // TODO: Support flags
                    // TODO: Expand environment-variables. TBD: Shall that be done on raw file, or after entries are parsed?

                    try result.entries.append(try ManifestEntry.create(line_idx, maybe_type, maybe_src, maybe_dst));
                }
            }
        }

        return result;
    }
};

test "Manifest.fromBuf" {
    // Empty
    {
        var buf =
            \\
        ;

        var manifest = try Manifest.fromBuf(buf);
        try testing.expectEqual(@as(usize, 0), manifest.len());
    }

    // Single entry, w comments
    {
        var buf =
            \\# git-repo from local path
            \\git: /my/remote/repo.git#main > ./path/to/local/clone
        ;
        var manifest = try Manifest.fromBuf(buf);
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
        var manifest = try Manifest.fromBuf(buf);
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
    
}


pub fn getLineEnding(buf: []const u8) []const u8 {
    if (std.mem.indexOf(u8, buf, "\r\n") != null) return "\r\n";
    return "\n";
}