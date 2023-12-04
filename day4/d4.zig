const std = @import("std");

const ScratchCard = struct {
  winning_numbers: std.ArrayList(u7),
  number_set: u128,

  fn scoreCard(self: ScratchCard) u32 {
    var num_matches = numMatches(self);
    if (num_matches == 0) return 0;
    return if (num_matches == 1) 1 else @as(u32, 1) << (num_matches - 1);
  }

  fn numMatches(self: ScratchCard) u5 {
    var count: u5 = 0;
    for (self.winning_numbers.items) |n| {
      if (self.number_set & (@as(u128, 1) << n) != 0) {
        count += 1;
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

  var scratch_cards = std.ArrayList(ScratchCard).init(arena.allocator());

  while (try f.reader().readUntilDelimiterOrEofAlloc(arena.allocator(), '\n', 200)) |line| {
    // Tokenise the line. We need to keep track of the winning numbers and the set
    // of numbers we have.
    // Represent the numbers we have using a bitset.
    var winning_numbers = std.ArrayList(u7).init(arena.allocator());

    var number_set: u128 = 0;

    var it = std.mem.split(u8, line, " ");
    var index: i32 = -1;
    while (it.next()) |tok| {
      if (tok.len == 0) continue;
      index += 1;
      if (index < 2) {
        continue;
      }
      else if (index < 12) {
        try winning_numbers.append(try std.fmt.parseInt(u7, tok, 10));
      }
      else if (index > 12) {
        number_set |= @as(u128, 1) << (try std.fmt.parseInt(u7, tok, 10));
      }
    }

    var sc: ScratchCard = .{.winning_numbers=winning_numbers, .number_set=number_set };
    try scratch_cards.append(sc);
  }

  var total1: u32 = 0;
  var total2: u32 = 0;

  var card_counts = [1]u32{1} ** 1000;
  for (scratch_cards.items, 0..) |sc, i| {
    var score = sc.scoreCard();
    total1 += score;
    total2 += card_counts[i];

    // Change card counts of subsequent cards.
    var num_matches = sc.numMatches();
    for (0..num_matches) |offset| {
      card_counts[i + 1 + offset] += card_counts[i];
    }
  }

  const stdout = std.io.getStdOut().writer();
  try stdout.print("P1: {d}, P2: {d}\n", .{total1, total2});
}
