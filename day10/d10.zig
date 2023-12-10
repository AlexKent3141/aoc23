const std = @import("std");
const stdout = std.io.getStdOut().writer();

const Direction = enum(u8) {
  Up = 0,
  Down = 1,
  Left = 2,
  Right = 3,
};

// Map each direction to its inverse.
const InverseDirection: [4]Direction = [_]Direction {
  Direction.Down, Direction.Up, Direction.Right, Direction.Left
};

const NeighbourGenerator = struct {
  // Centre of the neighbourhood.
  cx: i32,
  cy: i32,

  // String data giving us boundaries.
  rows: std.ArrayList([]u8),

  fn init(cx: i32, cy: i32, rows: std.ArrayList([]u8)) NeighbourGenerator {
    return .{.cx = cx, .cy = cy, .rows = rows};
  }

  fn get(self: NeighbourGenerator, d: Direction) ?[2]i32 {
    var x = self.cx;
    var y = self.cy;
    switch (d) {
      Direction.Right => {
        x = x + 1;
        if (x >= self.rows.items[0].len) return null;
      },
      Direction.Left => {
        x = x - 1;
        if (x < 0) return null;
      },
      Direction.Up => {
        y = y - 1;
        if (y < 0) return null;
      },
      Direction.Down => {
        y = y + 1;
        if (y >= self.rows.items.len) return null;
      },
    }

    return .{x, y};
  }
};

const Node = struct {
  c: u8,
  dirs: [2]Direction,

  fn init(c: u8) Node {
    var dirs: [2]Direction = switch (c) {
      '|' => [2]Direction{Direction.Up, Direction.Down},
      '-' => [2]Direction{Direction.Left, Direction.Right},
      'J' => [2]Direction{Direction.Left, Direction.Up},
      'L' => [2]Direction{Direction.Right, Direction.Up},
      'F' => [2]Direction{Direction.Down, Direction.Right},
      '7' => [2]Direction{Direction.Down, Direction.Left},
      else => unreachable,
    };

    return .{.c = c, .dirs = dirs};
  }

  fn route(self: Node, incoming: Direction) Direction {
    return if (self.dirs[0] == InverseDirection[@intFromEnum(incoming)]) self.dirs[1]
           else self.dirs[0];
  }

  fn has_incoming(self: Node, incoming: Direction) bool {
    return self.dirs[0] == incoming or self.dirs[1] == incoming;
  }
};

const Map = struct {
  rows: std.ArrayList([]u8),
  start_x: i32,
  start_y: i32,
  width: usize,
  on_path: []bool,

  fn init(rows: std.ArrayList([]u8), alloc: std.mem.Allocator) !Map {

    // Find the starting point.
    var start_x: usize = 0;
    var start_y: usize = 0;
    var found = false;

    for (rows.items, 0..) |r, y| {
      if (found) break;
      for (r, 0..) |c, x| {
        if (c != 'S') continue;
        start_x = x;
        start_y = y;
        found = true;
        break;
      }
    }

    return
    .{
      .rows = rows,
      .start_x = @intCast(start_x),
      .start_y = @intCast(start_y),
      .width = rows.items[0].len,
      .on_path = try alloc.alloc(bool, rows.items[0].len * rows.items.len),
    };
  }

  // Given an incoming direction, find the direction after traversing this location.
  fn next_loc(self: Map, current: [2]i32, d: *Direction) [2]i32 {
    var c = self.rows.items[@intCast(current[1])][@intCast(current[0])];
    d.* = Node.init(c).route(d.*);

    if (NeighbourGenerator.init(current[0], current[1], self.rows).get(d.*)) |loc| {
      return loc;
    }
    unreachable;
  }

  fn find_path(self: *Map) !u32 {
    // Work out where we're going first.
    var n: [2]i32 = undefined;
    var dirs = [1]Direction{Direction.Left} ** 2;
    var num_dirs: usize = 0;
    var neighbours = NeighbourGenerator.init(self.start_x, self.start_y, self.rows);

    for (0..4) |d| {
      const neighbour_dir: Direction = @enumFromInt(d);
      if (neighbours.get(neighbour_dir)) |loc| {
        var c = self.rows.items[@intCast(loc[1])][@intCast(loc[0])];
        if (Node.init(c).has_incoming(InverseDirection[@intFromEnum(neighbour_dir)])) {
          n = loc;
          dirs[num_dirs] = neighbour_dir;
          num_dirs = num_dirs + 1;
        }
      }
    }

    // Replace the start character.
    const possible_chars = "-FL7J|";
    var start_c: u8 = '.';
    for (possible_chars) |c| {
      var node = Node.init(c);
      if (node.has_incoming(dirs[0]) and node.has_incoming(dirs[1])) {
        start_c = c;
        break;
      }
    }

    self.rows.items[@intCast(self.start_y)][@intCast(self.start_x)] = start_c;

    @memset(self.on_path, false);

    // Set the start and first neighbour as being on the path.
    self.on_path[
      @as(usize, @intCast(self.start_y)) * self.width + @as(usize, @intCast(self.start_x))] = true;
    self.on_path[
      @as(usize, @intCast(n[1])) * self.width + @as(usize, @intCast(n[0]))] = true;

    // Do a loop from the first neighbour and see where we go.
    var path_length: u32 = 1;
    var current = n;
    var next = self.next_loc(current, &dirs[0]);
    path_length = path_length + 1;
    while (next[0] != self.start_x or next[1] != self.start_y) {
      self.on_path[
        @as(usize, @intCast(next[1])) * self.width + @as(usize, @intCast(next[0]))] = true;
      current = next;
      next = self.next_loc(current, &dirs[0]);
      path_length = path_length + 1;
    }

    return path_length;
  }

  fn count_enclosed(self: Map) u32 {
    var count: u32 = 0;
    for (self.rows.items, 0..) |row, y| {
      // Work across the row keeping track of parity.
      var total_crosses: u32 = 0;
      var state: ?u8 = null;
      for (row, 0..) |c, i| {
        if (!self.on_path[@as(usize, @intCast(y)) * self.width + i]) {
          if (total_crosses % 2 == 1) count = count + 1;
          continue;
        }

        if (c == '-') continue;
        if (c == '|') {
          total_crosses = total_crosses + 1;
          state = null;
          continue;
        }

        if (state) |s| {
          switch (c) {
            'J' => {
              switch (s) {
                'L' => total_crosses = total_crosses + 2,
                'F' => total_crosses = total_crosses + 1,
                else => unreachable,
              }
            },
            '7' => {
              switch (s) {
                'L' => total_crosses = total_crosses + 1,
                'F' => total_crosses = total_crosses + 2,
                else => unreachable,
              }
            },
            else => unreachable,
          }

          state = null;
        }
        else {
          state = c;
        }
      }
    }

    return count;
  }
};

pub fn main() !void {
  const f = try std.fs.cwd().openFile("input.txt", .{ .mode = std.fs.File.OpenMode.read_only });
  defer f.close();

  var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
  defer arena.deinit();

  var rows = std.ArrayList([]u8).init(arena.allocator());

  while (try f.reader().readUntilDelimiterOrEofAlloc(arena.allocator(), '\n', 200)) |line| {
    try rows.append(line);
  }

  var map = try Map.init(rows, arena.allocator());
  const farthest_distance = try map.find_path() / 2;

  // Got the path so we can now start assessing whether locations are inside it.
  const enclosed_tiles = map.count_enclosed();
  try stdout.print("P1: {d}, P2: {d}\n", .{farthest_distance, enclosed_tiles});
}
