#include "ctype.h"
#include "stdio.h"
#include "string.h"

#define MAX(x, y) (x > y ? x : y)

int main()
{
  FILE* f = fopen("input.txt", "r");
  char buf[200];
  int game_id;
  int game_id_total = 0;
  int game_power_total = 0;
  while (fscanf(f, "Game %d: %[^\n]\n", &game_id, buf) != EOF)
  {
    // Parsing the input is rather awkward (at least, it is in C).
    // Rather than the going down the `strtok` route we can get by
    // with a simple state machine.
    enum parser_state
    {
      NUMBER,
      COLOUR,
      NONE
    };

    int max_r = 0, max_g = 0, max_b = 0;
    enum parser_state state = NONE;
    int val = 0;
    for (size_t i = 0; i < strlen(buf); i++)
    {
      switch (state)
      {
        case NONE:
        {
          if (isdigit(buf[i]))
          {
            val = buf[i] - '0';
            state = NUMBER;
          }
          else if (isalpha(buf[i]))
          {
            // Completed an entry!
            switch (buf[i])
            {
              case 'r': max_r = MAX(max_r, val); break;
              case 'g': max_g = MAX(max_g, val); break;
              case 'b': max_b = MAX(max_b, val); break;
            }

            state = COLOUR;
          }
          break;
        }
        case NUMBER:
        {
          if (isdigit(buf[i]))
          {
            val *= 10;
            val += buf[i] - '0';
          }
          else
          {
            state = NONE;
          }
          break;
        }
        case COLOUR:
        {
          if (buf[i] == ' ')
          {
            state = NONE;
          }
          break;
        }
      }
    }

    // Are the maximums in range?
    if (max_r <= 12 && max_g <= 13 && max_b <= 14)
    {
      printf("%d is possible: %d %d %d\n", game_id, max_r, max_g, max_b);
      game_id_total += game_id;
    }

    game_power_total += max_r * max_g * max_b;
  }

  printf("P1: %d, P2: %d\n", game_id_total, game_power_total);

  return 0;
}
