const std = @import("std");
const stdout = std.io.getStdOut().writer();

const Star = struct {
  x: usize,
  y: usize,

  fn init(x: usize, y: usize) Star {
    return .{.x = x, .y = y};
  }
};

fn distance(
  s1: Star,
  s2: Star,
  expanding_rows: []bool,
  expanding_cols: []bool,
  expansion_factor: u64) u64 {

  // Manhattan distance metric with the proviso that crossing an expanding row/col
  // counts multiple times based on the expansion factor.
  var d: u64 = 0;
  const x_start: u64 = @min(s1.x, s2.x);
  const x_end: u64 = @max(s1.x, s2.x);

  for (x_start + 1..x_end + 1) |x| {
    d = d + 1;
    if (expanding_cols[x]) d = d + expansion_factor - 1;
  }

  // We're guaranteed that s2.y >= s1.y because of the order we're iterating.
  for (s1.y + 1..s2.y + 1) |y| {
    d = d + 1;
    if (expanding_rows[y]) d = d + expansion_factor - 1;
  }

  return d;
}

pub fn main() !void {
  const f = try std.fs.cwd().openFile("input.txt", .{ .mode = std.fs.File.OpenMode.read_only });
  defer f.close();

  var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
  defer arena.deinit();

  var rows = std.ArrayList([]u8).init(arena.allocator());
  while (try f.reader().readUntilDelimiterOrEofAlloc(arena.allocator(), '\n', 200)) |line| {
    try rows.append(line);
  }

  // Find the stars and the indices of rows and columns where expansion will occur.
  var stars = std.ArrayList(Star).init(arena.allocator());
  var empty_rows: []bool = try arena.allocator().alloc(bool, rows.items.len);
  var empty_cols: []bool = try arena.allocator().alloc(bool, rows.items[0].len);

  @memset(empty_rows, false);
  @memset(empty_cols, false);

  // We use the fact the input is square.
  for (rows.items, 0..) |r, y| {
    var row_is_empty = true;
    var col_is_empty = true;
    for (r, 0..) |c, x| {
      if (c == '#') try stars.append(Star.init(x, y));
      row_is_empty = row_is_empty and c == '.';
      col_is_empty = col_is_empty and rows.items[x][y] == '.';
    }

    empty_rows[y] = row_is_empty;
    empty_cols[y] = col_is_empty;
  }

  // Calculate distances between stars.
  var total1: u64 = 0;
  var total2: u64 = 0;
  for (0..stars.items.len) |i| {
    for (i + 1..stars.items.len) |j| {
      total1 = total1 + distance(stars.items[i], stars.items[j], empty_rows, empty_cols, 2);
      total2 = total2 + distance(stars.items[i], stars.items[j], empty_rows, empty_cols, 1e6);
    }
  }

  try stdout.print("P1: {d}, P2: {d}\n", .{total1, total2});
}
