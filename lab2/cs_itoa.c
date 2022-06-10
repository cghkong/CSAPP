#include<stdio.h>
#include<math.h>

void cs_itoa(int gh,char s[])
{
	int n,i;
	for ( n = 0; pow(10, n) <gh; n++)
	{
	}
	for ( i = 0; i < n; i++)
	{
		s[i] = (gh / ((int)(pow(10, n - i - 1))))%10 + '0';
	}
	s[n] = '\0';
}
int main()
{
	int gh=1614;
	char s[100];
	cs_itoa(gh,s);
	printf("%s\n",s);
	return 0;
}
