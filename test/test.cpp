/* LittleFall : Hello! */
#include <windows.h>
#include <stdio.h>
int main(void)
{
    __int16 a[] = {1, 2, 3, 4};
	__int16 b[] = {5, 6, 7, 8};
	__int16 c[4] = {0,0,0,0};
	__int16 fade = 127;
	//要完成 c = a>>8 + b
	_asm
    {
		movq mm0, a
		movq mm1, b
		paddsw mm0, mm1
		movq a, mm0
		emms
    }
    for(int i=0;i<4;++i)
    	printf("%d\n",a[i] );

    return 0;
}