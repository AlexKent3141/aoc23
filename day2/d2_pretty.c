          #include "stdio.h"
       #include      "string.h"
    #define MAX(x,y) (x > y ? x:y)
  #define ISD(c) ( c >='0'&&c<= '9')
 #define ISA(c) (c    >='a'&&c<= 'z')
int main(){FILE*       f=fopen("input"
".txt","r");char       buf[200];int gi;
int git=0;int         gpt = 0; while (
fscanf(   f,         "Game %d: %[^\n]\n"
,&gi,buf)!=        EOF){enum ps{NUM,COL,
  NONE};         int mr = 0, mg =0,mb=0
               ;enum ps s=NONE;int val=
             0;for(size_t i=0;i<strlen
           (buf);i++){switch(s){case
         NONE:{if(ISD(buf[i])){val
       = buf[i]-'0';s=NUM;} else
     if(ISA(buf[i])){ switch (
   buf[i]){case 'r': mr= MAX
 (mr,val);break;case'g':mg=
MAX(mg, val); break;case'b'
:mb=MAX(mb,val);break;}s=COL;}break;}case
NUM:{if(ISD(buf[i])){val*=10;val+= buf[i]
-'0';}else{s=NONE;}break;}case COL: { if(
buf[i]==' '){s = NONE; }break;}}}if(mr <=
12&&mg<=13&&mb<=14){git+=gi;}gpt +=mr*mg*
mb;}printf("P1: %d, P2: %d\n",git, gpt);}
