#include "ctype.h"
#include "stdio.h"
#include "string.h"

const char* numbers[] =
{
  "one", "two", "three", "four", "five", "six", "seven", "eight", "nine"
};

int get_digit_p1(char* p)
{
  if (isdigit(p[0])) return p[0] - '0';
  return 0;
}

int get_digit_p2(char* p)
{
  int p1 = get_digit_p1(p);
  if (p1) return p1;

  for (int i = 0; i < 9; i++)
  {
    if (strncmp(p, numbers[i], strlen(numbers[i])) == 0) return i + 1;
  }

  return 0;
}

int main()
{
  FILE* f = fopen("input.txt", "r");
  char buf[100];
  int total[2] = {0, 0};
  while (fscanf(f, "%s\n", buf) != EOF)
  {
    int digits_p1[2] = {0, 0};
    int digits_p2[2] = {0, 0};
    size_t len = strlen(buf);
    for (int i = 0; i < len; i++)
    {
      if (!digits_p1[0]) digits_p1[0] = get_digit_p1(&buf[i]);
      if (!digits_p2[0]) digits_p2[0] = get_digit_p2(&buf[i]);
      if (!digits_p1[1]) digits_p1[1] = get_digit_p1(&buf[len - i - 1]);
      if (!digits_p2[1]) digits_p2[1] = get_digit_p2(&buf[len - i - 1]);
    }

    total[0] += 10 * digits_p1[0] + digits_p1[1];
    total[1] += 10 * digits_p2[0] + digits_p2[1];
  }

  printf("P1: %d, P2: %d\n", total[0], total[1]);

  return 0;
}
