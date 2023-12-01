               #include "ctype.h"
              #include  "stdio.h"
             #include  "string.h"
            const char* numbers[]
          = {"one","two","three",
        "four",   "five",  "six",
      "seven","eight","nine"};int
    get_dp1(char* p) {if(isdigit(
  p[0]))return p[0]-'0';return 0;
} int get_dp2(char* p) { int p1 =
get_dp1(p); if (p1)return p1; for
(int i = 0;i<9; i++){if (strncmp(
           p, numbers[i], strlen(
           numbers[i]))==0)return
           i + 1;} return 0;} int
           main(){ FILE*f =fopen(
           "input.txt","r"); char
           buf[100];int total[2]=
           {0, 0};while(fscanf(f,
           "%s\n",buf)!= EOF){int
           dp1[2]={0, 0};int dp2[
           2] = {0, 0};size_t len
           = strlen(buf); for(int
           i = 0;i< len;i++) {if(
           !dp1[0])dp1[0]=get_dp1
           (&buf[i]);if (!dp2[0])
           dp2[0] = get_dp2(&buf[
           i]);if (!dp1[1])dp1[1]
           = get_dp1(&buf[len - i
           - 1]);if (!dp2[1]) dp2
[1]=get_dp2(&buf[len-i-1]);}total[0]+=10*dp1[0
]+dp1[1];total[1]+=10*dp2[0]+dp2[1];}printf("\
P1:%d, P2: %d\n",total[0],total[1]);return 0;}
