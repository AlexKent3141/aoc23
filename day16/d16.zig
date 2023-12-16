const std = @import("std");
const stdout = std.io.getStdOut().writer();

// Directions going anti-clockwise.
const Direction = enum(u3) {
  Up = 0,
  Left = 1,
  Down = 2,
  Right = 3
};

const InverseDirection = [4]Direction {
  Direction.Down, Direction.Right, Direction.Up, Direction.Left
};

const NeighbourGenerator = struct {
  // Centre of the neighbourhood.
  cx: i32,
  cy: i32,

  // String data giving us boundaries.
  rows: std.ArrayList([]const u8),

  fn init(cx: usize, cy: usize, rows: std.ArrayList([]const u8)) NeighbourGenerator {
    return .{.cx = @intCast(cx), .cy = @intCast(cy), .rows = rows};
  }

  fn get(self: NeighbourGenerator, d: Direction) ?[2]usize {
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

    return .{@intCast(x), @intCast(y)};
  }
};

const Reflector = struct {
  // For every incoming directions store the output directions.
  // Can have two outputs in the case of beam splitters.
  reflections: *const [4][2]?Direction,

  fn init(c: u8) Reflector {
    const h_split_directions = [4][2]?Direction {
      [2]?Direction{ Direction.Left, Direction.Right },
      [2]?Direction{ Direction.Right, null },
      [2]?Direction{ Direction.Left, Direction.Right },
      [2]?Direction{ Direction.Left, null }
    };

    const v_split_directions = [4][2]?Direction {
      [2]?Direction{ Direction.Down, null },
      [2]?Direction{ Direction.Up, Direction.Down},
      [2]?Direction{ Direction.Up, null },
      [2]?Direction{ Direction.Up, Direction.Down},
    };

    const bottom_left_to_upper_right_directions = [4][2]?Direction {
      [2]?Direction { Direction.Left, null },
      [2]?Direction { Direction.Up, null },
      [2]?Direction { Direction.Right, null },
      [2]?Direction { Direction.Down, null },
    };

    const upper_left_to_bottom_right_directions = [4][2]?Direction {
      [2]?Direction { Direction.Right, null },
      [2]?Direction { Direction.Down, null },
      [2]?Direction { Direction.Left, null },
      [2]?Direction { Direction.Up, null },
    };

    const reflections = switch (c) {
      '-'  => &h_split_directions,
      '|'  => &v_split_directions,
      '/'  => &bottom_left_to_upper_right_directions,
      '\\' => &upper_left_to_bottom_right_directions,
      else => unreachable,
    };

    return .{.reflections = reflections};
  }

  fn get_reflections(self: Reflector, dir: Direction) [2]?Direction {
    return self.reflections[@intFromEnum(InverseDirection[@intFromEnum(dir)])];
  }
};

const Map = struct {
  width: usize,
  height: usize,
  grid: std.ArrayList([]const u8),
  loc_directions: []u32,
  energised_count: u32 = 0,

  fn init(grid: std.ArrayList([]const u8), alloc: std.mem.Allocator) !Map {
    const width = grid.items[0].len;
    const height = grid.items.len;

    // For each location keep track of the direction we've been travelling as we passed through
    // as a bitset.
    const loc_directions = try alloc.alloc(u32, width * height);
    return .{.width = width, .height = height, .grid = grid, .loc_directions = loc_directions};
  }

  fn fire_laser_internal(self: *Map, x: usize, y: usize, dir: Direction) void {
    // Check whether we've alreayd passed through this location in the same direction.
    var loc_dirs = &self.loc_directions[y * self.width + x];

    if (loc_dirs.* & (@as(u32, 1) << @intFromEnum(dir)) != 0) {
      // We've already considered this path.
      return;
    }

    if (loc_dirs.* == 0) {
      self.energised_count = self.energised_count + 1;
    }

    loc_dirs.* = loc_dirs.* | (@as(u32, 1) << @intFromEnum(dir));

    // Consider step(s) from here.
    const n = NeighbourGenerator.init(x, y, self.grid);
    const c = self.grid.items[y][x];
    if (c == '.') {
      if (n.get(dir)) |next| {
        self.fire_laser_internal(next[0], next[1], dir);
      }

      return;
    }

    // Beam has encountered a reflector.
    const r = Reflector.init(c);
    const reflected_dirs = r.get_reflections(dir);
    for (reflected_dirs) |ref_dir| {
      if (ref_dir == null) continue;
      if (n.get(ref_dir.?)) |next| {
        self.fire_laser_internal(next[0], next[1], ref_dir.?);
      }
    }
  }

  // Consider a laserbeam entering with the specified location and direction.
  fn fire_laser(self: *Map, x: usize, y: usize, dir: Direction) void {
    self.energised_count = 0;
    @memset(self.loc_directions[0..], 0);
    self.fire_laser_internal(x, y, dir);
  }
};

pub fn main() !void {
  const f = try std.fs.cwd().openFile("input.txt", .{ .mode = std.fs.File.OpenMode.read_only });
  defer f.close();

  var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
  defer arena.deinit();

  var grid = std.ArrayList([]const u8).init(arena.allocator());

  const data = try f.reader().readAllAlloc(arena.allocator(), 30000);
  var it = std.mem.split(u8, data, "\n");
  while (it.next()) |line| {
    if (line.len == 0) continue;
    try grid.append(line);
  }

  var map = try Map.init(grid, arena.allocator());
  map.fire_laser(0, 0, Direction.Right);
  const p1 = map.energised_count;

  // For part 2 consider firing from every border location into the map.
  var best_energy: u32 = 0;

  // Assuming a square map.
  for (0..map.height) |s| {
    map.fire_laser(0, s, Direction.Right);
    best_energy = @max(best_energy, map.energised_count);

    map.fire_laser(map.height - 1, s, Direction.Left);
    best_energy = @max(best_energy, map.energised_count);

    map.fire_laser(s, 0, Direction.Down);
    best_energy = @max(best_energy, map.energised_count);

    map.fire_laser(s, map.height - 1, Direction.Up);
    best_energy = @max(best_energy, map.energised_count);
  }

  try stdout.print("P1 {d}, P2: {d}\n", .{p1, best_energy});
}
