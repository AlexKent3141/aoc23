const std = @import("std");

// For each location in the configuration and each group index store the number
// of possible valid states from there.
const DataCache = struct {
  cache: [][]?u64,

  fn init(config_len: usize, num_groups: usize, alloc: std.mem.Allocator) !DataCache {
    var cache = try alloc.alloc([]?u64, config_len);
    for (0..config_len) |i| {
      var group_row = try alloc.alloc(?u64, num_groups);
      @memset(group_row[0..], null);
      cache[i] = group_row;
    }

    return .{.cache = cache};
  }

  fn lookup(self: DataCache, config_index: u8, group_index: u8) ?u64 {
    return self.cache[config_index][group_index];
  }

  fn store(self: DataCache, config_index: u8, group_index: u8, val: u64) void {
    self.cache[config_index][group_index] = val;
  }
};

const DataSet = struct {
  config: []const u8,
  damaged_groups: std.ArrayList(u8),
  cache: DataCache,

  fn init(
    config: []const u8,
    damaged_groups: std.ArrayList(u8),
    alloc: std.mem.Allocator) !DataSet {

    return
    .{
      .config = config,
      .damaged_groups = damaged_groups,
      .cache = try DataCache.init(config.len, damaged_groups.items.len, alloc)
    };
  }

  fn duplicate(self: DataSet, factor: u8, alloc: std.mem.Allocator) !DataSet {
    // Allocate enough memory for the expanded configuration.
    var config = try alloc.alloc(u8, factor * (self.config.len + 1) - 1);
    for (0..factor) |i| {
      @memcpy(
        config[i*(self.config.len + 1)..(i + 1)*(self.config.len + 1) - 1],
        self.config[0..self.config.len]);

      if (i < factor - 1) {
        // Insert a '?'.
        config[(i + 1) * (self.config.len + 1) - 1] = '?';
      }
    }

    var damaged_groups = try self.damaged_groups.clone();
    for (0..factor - 1) |_| {
      for (self.damaged_groups.items) |d| {
        try damaged_groups.append(d);
      }
    }

    return DataSet.init(config, damaged_groups, alloc);
  }

  fn count_internal(self: DataSet, config_index: u8, group_index: u8) u64 {
    // If all groups have been placed check that we haven't left any
    // known unsatisfied bad springs.
    if (group_index == self.damaged_groups.items.len) {
      if (config_index < self.config.len) {
        for (config_index..self.config.len) |i| {
          if (self.config[i] == '#') {
            return 0;
          }
        }
      }

      // We've got a possible configuration!
      return 1;
    }

    // Check that the current group would fit without overrunning the config.
    const group_size = self.damaged_groups.items[group_index];
    if (config_index + group_size > self.config.len) {
      // Off the end of the grid, so impossible.
      return 0;
    }

    // Check for a cached result for this state.
    if (self.cache.lookup(config_index, group_index)) |val| {
      return val;
    }

    // Skip over known good springs.
    if (self.config[config_index] == '.') {
      return count_internal(self, config_index + 1, group_index);
    }

    var total: u64 = 0;

    // If it's a '?' then consider not placing here.
    if (self.config[config_index] == '?')
    {
      total = count_internal(self, config_index + 1, group_index);
    }

    // We're in the case where we place.

    // Check that the group wouldn't overlap with any known good springs.
    for (1..group_size) |i| {
      if (self.config[config_index + i] == '.') {
        // This is impossible, must be in the no placement case.
        return total;
      }
    }

    // Final check: if the next character after the group is a '#' then we
    // can't place here because that is a known bad spring and it would
    // extend the group.
    if (config_index + group_size < self.config.len) {
      if (self.config[config_index + group_size] == '#') {
        return total;
      }
    }

    // We can place the group here.
    total += count_internal(self, config_index + group_size + 1, group_index + 1);

    self.cache.store(config_index, group_index, total);

    return total;
  }

  fn count(self: DataSet) u64 {
    return count_internal(self, 0, 0);
  }
};

pub fn main() !void {
  const f = try std.fs.cwd().openFile("input.txt", .{ .mode = std.fs.File.OpenMode.read_only });
  defer f.close();

  var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
  defer arena.deinit();

  var data = std.ArrayList(DataSet).init(arena.allocator());

  while (try f.reader().readUntilDelimiterOrEofAlloc(arena.allocator(), '\n', 100)) |line| {

    var damaged_groups = std.ArrayList(u8).init(arena.allocator());

    var it = std.mem.split(u8, line, " ");

    const config = it.next() orelse unreachable;
    const damaged_groups_list = it.next() orelse unreachable;

    var groups_it = std.mem.split(u8, damaged_groups_list, ",");
    while (groups_it.next()) |tok| {
      try damaged_groups.append(try std.fmt.parseInt(u8, tok, 10));
    }

    try data.append(try DataSet.init(config, damaged_groups, arena.allocator()));
  }

  var total1: u64 = 0;
  var total2: u64 = 0;
  for (data.items) |d| {
    var poss = d.count();
    total1 += poss;

    const dup = try d.duplicate(5, arena.allocator());
    poss = dup.count();
    total2 += poss;
  }

  const stdout = std.io.getStdOut().writer();
  try stdout.print("P1 {d}, P2: {d}\n", .{total1, total2});
}
