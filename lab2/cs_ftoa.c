 #include<stdio.h>
 #include <math.h>
 #include <stdlib.h>

 const double eps = 1e-11;

 void cs_ftoa(double num,char *str)
 {
     int hg;
     double lw;
     char *start=str;
     int n=0;
     char ch[20];
     int i;
     hg=(int)num;
     lw=num-hg;

     while(hg>0){
         ch[n++]='0'+hg%10;
         hg=hg/10;
     }

     for(i=n-1;i>=0;i--){
         *str++=ch[i];
     }

     num -= (int)num;
    double tp = 0.1;
     *str++='.';

     while(num > eps){
         num -= tp * (int)(lw * 10);
         tp /= 10;
         *str++='0'+(int)(lw*10);
         lw=lw*10.0-(int)(lw*10);
     }
     *str='\0';
     str=start;
 }
 int main()
 {
    double a=1.3456;
    char str[50];
    cs_ftoa(a,str);
    printf("%s\n",str);
    return 0;
 }
