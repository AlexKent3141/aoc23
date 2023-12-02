const std = @import("std");

const numbers: [9][]const u8 = [_][]const u8{
  "one", "two", "three", "four", "five", "six", "seven", "eight", "nine"
};

pub fn get_digit_p1(buf: []u8) u32 {
  if (std.ascii.isDigit(buf[0])) return buf[0] - '0';
  return 0;
}

pub fn get_digit_p2(buf: []u8) u32 {
  var p1: u32 = get_digit_p1(buf);
  if (p1 != 0) return p1;

  for (numbers, 0..) |n, i| {
    if (std.mem.startsWith(u8, buf, n)) return @as(u32, @intCast(i + 1));
  }

  return 0;
}

pub fn main() !void
{
  const f = try std.fs.cwd().openFile("input.txt", .{ .mode = std.fs.File.OpenMode.read_only });
  defer f.close();

  var buf: [100]u8 = undefined;
  var totals = [2]u32{0, 0};
  while (try f.reader().readUntilDelimiterOrEof(&buf, '\n')) |line| {
    var digits1 = [2]u32{0, 0};
    var digits2 = [2]u32{0, 0};
    for (line, 0..) |_, i| {
      if (digits1[0] == 0) digits1[0] = get_digit_p1(line[i..line.len]);
      if (digits1[1] == 0) digits1[1] = get_digit_p1(line[line.len-i-1..line.len]);
      if (digits2[0] == 0) digits2[0] = get_digit_p2(line[i..line.len]);
      if (digits2[1] == 0) digits2[1] = get_digit_p2(line[line.len-i-1..line.len]);
    }
    totals[0] += 10 * digits1[0] + digits1[1];
    totals[1] += 10 * digits2[0] + digits2[1];
  }
  const stdout = std.io.getStdOut().writer();
  try stdout.print("P1: {d}, P2: {d}\n", .{totals[0], totals[1]});
}
