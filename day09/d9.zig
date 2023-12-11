const std = @import("std");
const stdout = std.io.getStdOut().writer();

const DataSet = struct {
  data: std.ArrayList(i32) = undefined,

  fn init(data: std.ArrayList(i32)) DataSet {
    return .{.data = data };
  }
  
  fn predict(self: DataSet, comptime Alloc: type, alloc: *Alloc) ![2]i32 {
    var rows = std.ArrayList(std.ArrayList(i32)).init(alloc.allocator());
    try rows.append(self.data);

    // Keep generating new rows until all elements are zero.
    var is_zero = false;
    while (!is_zero) {
      is_zero = true;
      var prev_row = rows.items[rows.items.len - 1];
      var next_row = std.ArrayList(i32).init(alloc.allocator());
      for (1..prev_row.items.len) |i| {
        var diff = prev_row.items[i] - prev_row.items[i - 1];
        try next_row.append(diff);
        is_zero = is_zero and diff == 0;
      }

      try rows.append(next_row);
    }

    try stdout.print("Num rows needed: {d}\n", .{rows.items.len});

    // Backtrack to make the predictions.
    // The prediction at index 0 is in the past, 1 is in the future.
    var pred = [2]i32{0, 0};
    for (0..rows.items.len - 1) |i| {
      var prev_row = rows.items[rows.items.len - i - 2];
      pred[0] = prev_row.items[0] - pred[0];
      pred[1] = pred[1] + prev_row.items[prev_row.items.len - 1];
    }

    return pred;
  }
};

pub fn main() !void {
  const f = try std.fs.cwd().openFile("input.txt", .{ .mode = std.fs.File.OpenMode.read_only });
  defer f.close();

  var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
  defer arena.deinit();

  var datasets = std.ArrayList(DataSet).init(arena.allocator());

  while (try f.reader().readUntilDelimiterOrEofAlloc(arena.allocator(), '\n', 200)) |line| {
    var it = std.mem.split(u8, line, " ");

    var data = std.ArrayList(i32).init(arena.allocator());
    while (it.next()) |tok| {
      try data.append(try std.fmt.parseInt(i32, tok, 10));
    }

    try datasets.append(DataSet.init(data));
  }

  var total = [2]i32{0, 0};
  for (datasets.items) |ds| {
    for (ds.data.items) |d| {
      try stdout.print("{d} ", .{d});
    }
    try stdout.print("Dataset of size: {d}\n", .{ds.data.items.len});
    var pred = try ds.predict(std.heap.ArenaAllocator, &arena);
    try stdout.print("Prediction: {d}\n", .{pred});
    total[0] = total[0] + pred[0];
    total[1] = total[1] + pred[1];
  }

  try stdout.print("P1: {d}, P2: {d}\n", .{total[1], total[0]});
}
