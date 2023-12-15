const std = @import("std");
const stdout = std.io.getStdOut().writer();

const HashEntry = struct {
  label: []const u8,
  value: ?u8,
  needs_to_be_removed: bool,
  has_been_removed: bool,

  fn init(s: []const u8) HashEntry {
    var label: []const u8 = undefined;
    var needs_to_be_removed = false;
    var value: ?u8 = null;
    if (s[s.len - 1] == '-') {
      label = s[0..s.len - 1];
      needs_to_be_removed = true;
    }
    else {
      label = s[0..s.len - 2];
      value = s[s.len - 1] - '0';
    }

    return
    .{
      .label = label,
      .value = value,
      .needs_to_be_removed = needs_to_be_removed,
      .has_been_removed = false
    };
  }
};

fn hash(s: []const u8) u64 {
  var val: u64 = 0;
  for (s) |c| {
    val = val + c;
    val = val * 17;
    val = val % 256;
  }

  return val;
}

pub fn main() !void {
  const f = try std.fs.cwd().openFile("input.txt", .{ .mode = std.fs.File.OpenMode.read_only });
  defer f.close();

  var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
  defer arena.deinit();

  var data = std.ArrayList([]const u8).init(arena.allocator());
  var line = try f.reader().readAllAlloc(arena.allocator(), 30000);
  var it = std.mem.split(u8, line[0..line.len - 1], ",");
  while (it.next()) |tok| {
    try data.append(tok);
  }

  var total1: u64 = 0;
  for (data.items) |s| {
    var h = hash(s);
    total1 = total1 + h;
  }

  var buckets = [1]std.ArrayList(HashEntry){std.ArrayList(HashEntry).init(arena.allocator())} ** 256;

  for (data.items) |s| {
    var entry = HashEntry.init(s);
    var h = hash(entry.label);

    // Process this entry.
    var bucket = &buckets[h];
    if (entry.needs_to_be_removed) {
      for (0..bucket.items.len) |i| {
        if (std.mem.eql(u8, bucket.items[i].label, entry.label)) {
          bucket.items[i].has_been_removed = true;
        }
      }
    }
    else {
      var modified = false;
      for (0..bucket.items.len) |i| {
        if (bucket.items[i].has_been_removed) continue;

        if (std.mem.eql(u8, bucket.items[i].label, entry.label)) {
          bucket.items[i].value = entry.value;
          bucket.items[i].has_been_removed = false;
          modified = true;
          break;
        }
      }

      if (!modified) {
        try bucket.append(entry);
      }
    }
  }

  // Hopefully have all the info I need now.
  var total2: u64 = 0;
  for (0..256) |bucket_index| {
    const bucket = buckets[bucket_index];
    var true_index: usize = 0;
    for (bucket.items) |entry| {
      if (entry.has_been_removed) continue;
      true_index = true_index + 1;
      var prod = (bucket_index + 1) * true_index * entry.value.?;
      total2 = total2 + prod;
    }
  }

  try stdout.print("P1 {d}, P2: {d}\n", .{total1, total2});
}
