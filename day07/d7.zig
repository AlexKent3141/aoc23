const std = @import("std");

const HandType = enum {
  HighCard,
  OnePair,
  TwoPair,
  ThreeOfAKind,
  FullHouse,
  FourOfAKind,
  FiveOfAKind,
};

const Hand = struct {
  cards: []const u8,
  bid: u32,
  hand_type: HandType,

  // Additions for P2.
  hand_type_with_joker: HandType,

  fn init(cards: []const u8, bid: u32) Hand {
    // Classify the hand type here.
    // Count the number of pairs, triples, quadruples etc.
    var counts = [1]u8{0} ** 15;
    var num_jokers: u8 = 0;
    for (cards) |c| {
      counts[c] += 1;
      if (c == 11) num_jokers += 1;
    }

    var num_pairs: u8 = 0;
    var num_triples: u8 = 0;
    var num_quads: u8 = 0;
    var num_pents: u8 = 0;

    for (counts) |c| {
      switch (c) {
        2 => num_pairs += 1,
        3 => num_triples += 1,
        4 => num_quads += 1,
        5 => num_pents += 1,
        else => continue,
      }
    }

    // Now work out the hand types with and without jokers.
    var hand_type = HandType.HighCard;
    var hand_type_with_joker = HandType.HighCard;
    if (num_pents == 1) {
      hand_type = HandType.FiveOfAKind;
    }
    else if (num_quads == 1) {
      hand_type = HandType.FourOfAKind;

      if (num_jokers == 1 or num_jokers == 4) {
        hand_type_with_joker = HandType.FiveOfAKind;
      }
    }
    else if (num_triples == 1 and num_pairs == 1) {
      hand_type = HandType.FullHouse;

      if (num_jokers > 0) {
        hand_type_with_joker = HandType.FiveOfAKind;
      }
    }
    else if (num_triples == 1) {
      hand_type = HandType.ThreeOfAKind;

      if (num_jokers == 1 or num_jokers == 3) {
        hand_type_with_joker = HandType.FourOfAKind;
      }
    }
    else if (num_pairs == 2) {
      hand_type = HandType.TwoPair;

      if (num_jokers == 1) {
        hand_type_with_joker = HandType.FullHouse;
      }
      else if (num_jokers == 2) {
        hand_type_with_joker = HandType.FourOfAKind;
      }
    }
    else if (num_pairs == 1) {
      hand_type = HandType.OnePair;

      if (num_jokers == 1 or num_jokers == 2) {
        hand_type_with_joker = HandType.ThreeOfAKind;
      }
    }
    else if (num_jokers == 1) {
      hand_type_with_joker = HandType.OnePair;
    }

    if (@intFromEnum(hand_type_with_joker) < @intFromEnum(hand_type)) {
      hand_type_with_joker = hand_type;
    }

    return
    .{
      .cards = cards,
      .bid = bid,
      .hand_type = hand_type,
      .hand_type_with_joker = hand_type_with_joker
    };
  }
};

fn cmpHandsP1(context: void, h1: Hand, h2: Hand) bool {
  if (h1.hand_type != h2.hand_type) {
    return @intFromEnum(h1.hand_type) < @intFromEnum(h2.hand_type);
  }

  for (0..5) |i| {
    if (h1.cards[i] != h2.cards[i]) {
      return h1.cards[i] < h2.cards[i];
    }
  }

  _ = context;

  return false;
}

fn cmpHandsP2(context: void, h1: Hand, h2: Hand) bool {
  // If one hand clearly outranks the other it's easy.
  if (h1.hand_type_with_joker != h2.hand_type_with_joker) {
    return @intFromEnum(h1.hand_type_with_joker) < @intFromEnum(h2.hand_type_with_joker);
  }

  // Fall back on card comparison, remembering that joker is weakest.
  for (0..5) |i| {
    if (h1.cards[i] != h2.cards[i]) {
      if (h1.cards[i] == 11) {
        return true;
      }
      else if (h2.cards[i] == 11) {
        return false;
      }

      return h1.cards[i] < h2.cards[i];
    }
  }

  _ = context;

  return false;
}

pub fn main() !void {
  const f = try std.fs.cwd().openFile("input.txt", .{ .mode = std.fs.File.OpenMode.read_only });
  defer f.close();

  var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
  defer arena.deinit();

  var hands = std.ArrayList(Hand).init(arena.allocator());

  while (try f.reader().readUntilDelimiterOrEofAlloc(arena.allocator(), '\n', 50)) |line| {
    var it = std.mem.split(u8, line, " ");

    var cards: []u8 = try arena.allocator().alloc(u8, 5);

    var optional_tok = it.next();
    if (optional_tok) |tok| {
      std.mem.copy(u8, cards, tok);

      // Remap the letter representations to numbers for easier comparison.
      for (cards, 0..) |c, i| {
        cards[i] = switch (c) {
          '2'...'9' => c - '0',
          'T' => 10,
          'J' => 11,
          'Q' => 12,
          'K' => 13,
          'A' => 14,
          else => unreachable,
        };
      }
    }

    var bid = try std.fmt.parseInt(u32, it.next() orelse "0", 10);

    try hands.append(Hand.init(cards, bid));
  }

  std.sort.insertion(Hand, hands.items, {}, cmpHandsP1);

  const stdout = std.io.getStdOut().writer();
  var total1: usize = 0;
  for (hands.items, 0..) |hand, i| {
    var cards = hand.cards;
    try stdout.print(
      "{d} {d} {d} {d} {d}, {d}\n",
      .{cards[0], cards[1], cards[2], cards[3], cards[4], hand.bid});

    total1 += (i + 1) * hand.bid;
  }

  std.sort.insertion(Hand, hands.items, {}, cmpHandsP2);

  var total2: usize = 0;
  for (hands.items, 0..) |hand, i| {
    var cards = hand.cards;
    try stdout.print(
      "{d} {d} {d} {d} {d}, {d}\n",
      .{cards[0], cards[1], cards[2], cards[3], cards[4], hand.bid});

    total2 += (i + 1) * hand.bid;
  }

  try stdout.print("P1: {d}, P2: {d}\n", .{total1, total2});
}
