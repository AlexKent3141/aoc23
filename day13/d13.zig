const std = @import("std");
const stdout = std.io.getStdOut().writer();

const Orientation = enum {
  Horizontal,
  Vertical
};

const Line = struct {
  orientation: Orientation,
  index: usize,

  fn init(orientation: Orientation, index: usize) Line {
    return .{.orientation = orientation, .index = index};
  }
};

const Pattern = struct {
  width: usize,
  height: usize,
  grid: std.ArrayList([]u8),

  fn init(grid: std.ArrayList([]u8)) Pattern {
    return .{.width = grid.items[0].len, .height = grid.items.len, .grid = grid};
  }

  fn symmetries(self: Pattern, alloc: std.mem.Allocator) !std.ArrayList(Line) {
    var syms = std.ArrayList(Line).init(alloc);

    // Iterate over cols.
    for (1..self.width) |col| {
      var is_symmetric = true;
      no_sym: for (self.grid.items) |row| {
        // Check whether this row is symmetric around this col.
        for (0..col) |j| {
          // Would this col reflect to a non-existent location?
          var reflected_loc = col + (col - j) - 1;
          if (reflected_loc >= self.width) continue;
          if (row[j] != row[reflected_loc]) {
            is_symmetric = false;
            break :no_sym;
          }
        }
      }

      if (is_symmetric) try syms.append(Line.init(Orientation.Horizontal, col));
    }

    // Iterate over rows.
    for (1..self.height) |row| {
      var is_symmetric = true;
      no_sym: for (0..self.grid.items[0].len) |col| {
        // Check whether this column is symmetric around this row.
        for (0..row) |i| {
          // Would this row reflect to a non-existent location?
          var reflected_loc = row + (row - i) - 1;
          if (reflected_loc >= self.height) continue;
          if (self.grid.items[i][col] != self.grid.items[reflected_loc][col]) {
            is_symmetric = false;
            break :no_sym;
          }
        }
      }

      if (is_symmetric) try syms.append(Line.init(Orientation.Vertical, row));
    }

    return syms;
  }

  fn flip(self: Pattern, x: usize, y: usize) void {
    const c = self.grid.items[y][x];
    self.grid.items[y][x] = switch (c) {
      '.' => '#',
      '#' => '.',
      else => unreachable,
    };
  }

  fn symmetry_without_smudge(self: Pattern, alloc: std.mem.Allocator) !Line {
    // Get initial symmetries.
    const initial_syms = try self.symmetries(alloc);
    const initial_line = initial_syms.items[0];

    // Try removing the smudge in every possible location.
    for (0..self.width) |x| {
      for (0..self.height) |y| {
        // Flip this one.
        self.flip(x, y);

        const next_syms = try self.symmetries(alloc);
        self.flip(x, y);

        // Are any of the new symemtries different to the initial one?
        for (next_syms.items) |line| {
          if (!std.meta.eql(line, initial_line)) {
            return line;
          }
        }
      }
    }

    unreachable;
  }
};

pub fn main() !void {
  const f = try std.fs.cwd().openFile("input.txt", .{ .mode = std.fs.File.OpenMode.read_only });
  defer f.close();

  var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
  defer arena.deinit();

  var patterns = std.ArrayList(Pattern).init(arena.allocator());

  var current_grid = std.ArrayList([]u8).init(arena.allocator());

  while (try f.reader().readUntilDelimiterOrEofAlloc(arena.allocator(), '\n', 100)) |line| {

    if (line.len == 0) {
      // Complete pattern.
      try patterns.append(Pattern.init(current_grid));
      current_grid = std.ArrayList([]u8).init(arena.allocator());
      continue;
    }

    try current_grid.append(line);
  }

  try patterns.append(Pattern.init(current_grid));

  var total1: usize = 0;
  var total2: usize = 0;
  for (patterns.items, 0..) |p, i| {
    try stdout.print("Pattern: {d}\n", .{i});
    for (p.grid.items) |row| {
      try stdout.print("{s}\n", .{row});
    }

    // These should have exactly one symmetry.
    const initial_syms = try p.symmetries(arena.allocator());
    const line = initial_syms.items[0];

    total1 = total1 + switch (line.orientation) {
      Orientation.Horizontal => line.index,
      Orientation.Vertical => 100 * line.index,
    };

    // Consider flipping each location until we find a configuration giving different symmetries.
    const unsmudged_line = try p.symmetry_without_smudge(arena.allocator());

    total2 = total2 + switch (unsmudged_line.orientation) {
      Orientation.Horizontal => unsmudged_line.index,
      Orientation.Vertical => 100 * unsmudged_line.index,
    };

    try stdout.print("Initial line index: {d}\n", .{line.index});
    try stdout.print("Unsmudged line index: {d}\n", .{unsmudged_line.index});
  }

  try stdout.print("P1 {d}, P2: {d}\n", .{total1, total2});
}
