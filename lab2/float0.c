#include<stdio.h>
int main()
{
	float a=3,minf;
	printf("除以0  %f\n", a / 0);
	char* x;
	x = (char*)& minf;
	*x = 0b00000001;
	*(x + 1) = 0b00000000;
	*(x + 2) = 0b00000000;
	*(x + 3) = 0b00000000;
	printf("除以极小浮点数  %f", a / minf);
}
