const std = @import("std");
const stdout = std.io.getStdOut().writer();

// Pairs of offsets.
const offsets = [8][2]i32{
  [2]i32 { -1, -1 }, [2]i32 { -1,  0 }, [2]i32 { -1, 1 }, [2]i32 { 1,  0 },
  [2]i32 {  1,  1 }, [2]i32 {  1, -1 }, [2]i32 {  0, 1 }, [2]i32 { 0, -1 }
};

const NeighbourIterator = struct {
  // Centre of the neighbourhood.
  cx: i32,
  cy: i32,

  // String data giving us boundaries.
  lines: std.ArrayList([]u8),

  // Current iterator state.
  s: u32 = 0,

  fn next(self: *NeighbourIterator) ?[2]usize{
    while (self.s < 8) {
      self.s += 1;
      var x: i32 = self.cx + offsets[self.s - 1][0];
      var y: i32 = self.cy + offsets[self.s - 1][1];
      if (x < 0 or x >= self.lines.items[0].len or y < 0 or y >= self.lines.items.len) continue;
      return .{@intCast(x), @intCast(y)};
    }

    return null;
  }
};

pub fn isSymbol(c: u8) bool {
  return c != '.' and !std.ascii.isDigit(c);
}

pub fn isAdjacentToSymbol(x: i32, y: i32, lines: std.ArrayList([]u8)) !bool {
  var g: NeighbourIterator = .{.cx = x, .cy = y, .lines = lines };
  while (g.next()) |loc| {
    if (isSymbol(lines.items[loc[1]][loc[0]])) return true;
  }

  return false;
}

const NumEntry = struct {
  n: u32,
  id: i32
};

pub fn gearRatio(x: i32, y: i32, lines: std.ArrayList([]u8), numbers: std.ArrayList(NumEntry)) !u32 {
  var prod: u32 = 1;

  var considered_ids: [3]i32 = [_]i32{ -1, -1, -1 };
  var num_considered: i32 = 0;
  var g: NeighbourIterator = .{.cx = x, .cy = y, .lines = lines };
  while (g.next()) |loc| {
    var index: usize = loc[1] * lines.items[0].len + loc[0];
    var entry: NumEntry = numbers.items[index];

    for (0..3) |k| {
      if (considered_ids[k] == entry.id) {
        // No need to consider this one: we've already included it.
        break;
      }
      if (considered_ids[k] == -1) {
        // Got an empty space in the cache: fill it with this entry.
        considered_ids[k] = entry.id;
        num_considered += 1;
        prod *= entry.n;
        break;
      }
    }
  }

  if (num_considered == 2) {
    return prod;
  }

  return 0;
}

pub fn main() !void {
  const f = try std.fs.cwd().openFile("input.txt", .{ .mode = std.fs.File.OpenMode.read_only });
  defer f.close();

  // Read all lines into an ArrayList.
  var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
  defer arena.deinit();
  var list = std.ArrayList([]u8).init(arena.allocator());

  while (try f.reader().readUntilDelimiterOrEofAlloc(arena.allocator(), '\n', 200)) |line| {
    try list.append(line);
  }

  // First pass: calculate part 1 and store every complete number at each location.
  var numbers_list = std.ArrayList(NumEntry).init(arena.allocator());
  var total1: u32 = 0;
  var num_id: i32 = 0;
  for (list.items, 0..) |line, y_| {
    var i: usize = 0;
    while (i < line.len) : (i += 1) {
      if (!std.ascii.isDigit(line[i]))
      {
        try numbers_list.append(NumEntry{.n=1, .id=-1});
        continue;
      }

      const start_x: u64 = i;
      var n: u32 = 0;
      var is_adjacent: bool = false;
      while (i < line.len and std.ascii.isDigit(line[i])) {
        is_adjacent = is_adjacent or try isAdjacentToSymbol(@intCast(i), @intCast(y_), list);
        n *= 10;
        n += line[i] - '0';
        i += 1;
      }

      if (is_adjacent) total1 += n;

      // Store the complete number at each index that it exists.
      for (start_x..i) |_| try numbers_list.append(NumEntry{.n=n, .id=num_id});
      num_id += 1;

      // We've already iterated past the number so ensure we have an entry for this char.
      if (i < line.len) try numbers_list.append(NumEntry{.n=1, .id=-1});
    }
  }

  // Second pass: find the '*' symbols and work out gear ratios.
  var total2: u64 = 0;
  for (list.items, 0..) |line, y| {
    for (line, 0..) |c, x| {
      if (c != '*') continue;

      var r: u32 = try gearRatio(@intCast(x), @intCast(y), list, numbers_list);
      if (r != 0) {
        total2 += r;
      }
    }
  }

  try stdout.print("P1: {d}, P2: {d}\n", .{total1, total2});
}
