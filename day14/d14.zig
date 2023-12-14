const std = @import("std");
const stdout = std.io.getStdOut().writer();

const Map = struct {
  width: usize,
  height: usize,
  grid: std.ArrayList([]u8),

  fn init(grid: std.ArrayList([]u8)) Map {
    return .{.width = grid.items[0].len, .height = grid.items.len, .grid = grid};
  }

  fn tilt_north(self: Map) !void {
    // Iterate across columns modifying the grid so that each rock is as far north as possible.
    for (0..self.width) |col| {
      var current_top: usize = 0;
      for (0..self.height) |row| {
        switch (self.grid.items[row][col]) {
          '.' => continue,
          '#' => current_top = row + 1,
          'O' => {
            if (current_top != row) {
              self.grid.items[current_top][col] = 'O';
              self.grid.items[row][col] = '.';
            }

            current_top = current_top + 1;
          },
          else => unreachable,
        }
      }
    }
  }

  fn tilt_south(self: Map) !void {
    for (0..self.width) |col| {
      var current_bottom: i32 = @intCast(self.height - 1);
      for (0..self.height) |r| {
        var row: i32 = @intCast(self.height - 1 - r);
        switch (self.grid.items[@intCast(row)][col]) {
          '.' => continue,
          '#' => current_bottom = row - 1,
          'O' => {
            if (current_bottom != row) {
              self.grid.items[@intCast(current_bottom)][col] = 'O';
              self.grid.items[@intCast(row)][col] = '.';
            }

            current_bottom = current_bottom - 1;
          },
          else => unreachable,
        }
      }
    }
  }

  fn tilt_east(self: Map) !void {
    for (0..self.height) |row| {
      var current_right: i32 = @intCast(self.width - 1);
      for (0..self.width) |c| {
        var col: i32 = @intCast(self.width - 1 - c);
        switch (self.grid.items[row][@intCast(col)]) {
          '.' => continue,
          '#' => current_right = col - 1,
          'O' => {
            if (current_right != col) {
              self.grid.items[row][@intCast(current_right)] = 'O';
              self.grid.items[row][@intCast(col)] = '.';
            }

            current_right = current_right - 1;
          },
          else => unreachable,
        }
      }
    }
  }

  fn tilt_west(self: Map) !void {
    for (0..self.height) |row| {
      var current_left : usize = 0;
      for (0..self.width) |col| {
        switch (self.grid.items[row][col]) {
          '.' => continue,
          '#' => current_left = col + 1,
          'O' => {
            if (current_left != col) {
              self.grid.items[row][current_left] = 'O';
              self.grid.items[row][col] = '.';
            }

            current_left = current_left + 1;
          },
          else => unreachable,
        }
      }
    }
  }

  fn count_load(self: Map) usize {
    var total: usize = 0;
    for (0..self.width) |col| {
      for (0..self.height) |row| {
        if (self.grid.items[row][col] == 'O') {
          total += self.height - row;
        }
      }
    }

    return total;
  }
};

fn detect_period(load_counts: std.ArrayList(usize)) ?usize {
  // Only start checking when we have > 1000 points.
  const min_loads: usize = 1000;
  if (load_counts.items.len < min_loads) return null;

  // Try periods from 1 up to 100.
  for (1..101) |period| {
    const num_periods_to_check: usize = min_loads / period;
    var is_periodic = true;
    no_period: for (0..period) |i| {
      // This index must match across all of the in-range periods.
      for (1..num_periods_to_check) |period_index| {
        // Work backwards through the input.
        if (load_counts.items[load_counts.items.len - i - 1] !=
            load_counts.items[load_counts.items.len - (i + period * period_index) - 1]) {
          is_periodic = false;
          break :no_period;
        }
      }
    }

    if (is_periodic) {
      return period;
    }
  }

  return null;
}

pub fn main() !void {
  const f = try std.fs.cwd().openFile("input.txt", .{ .mode = std.fs.File.OpenMode.read_only });
  defer f.close();

  var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
  defer arena.deinit();

  var grid = std.ArrayList([]u8).init(arena.allocator());

  while (try f.reader().readUntilDelimiterOrEofAlloc(arena.allocator(), '\n', 200)) |line| {
    try grid.append(line);
  }

  const map = Map.init(grid);

  var p1: usize = 0;
  var num_cycles: u32 = 0;
  var load_counts = std.ArrayList(usize).init(arena.allocator());
  var period: ?usize = null;

  while (period == null) {
    try map.tilt_north();
    if (p1 == 0) p1 = map.count_load();
    try map.tilt_west();
    try map.tilt_south();
    try map.tilt_east();

    num_cycles = num_cycles + 1;

    const load = map.count_load();
    try load_counts.append(load);

    try stdout.print("{d} Load: {d}\n", .{num_cycles, load});

    period = detect_period(load_counts);
  }

  try stdout.print("Period is: {?}\n", .{period});

  var period_index = (1000000000 - load_counts.items.len) % period.?;
  try stdout.print("Period index is: {d}\n", .{period_index});

  var period_slice = load_counts.items[load_counts.items.len - period.? - 1..];
  try stdout.print("Period slice is: {d}\n", .{period_slice});

  try stdout.print("P1 {d}, P2: {d}\n", .{p1, period_slice[period_index]});
}
