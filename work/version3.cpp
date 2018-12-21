#include <windows.h>
#include <stdio.h>

const char *filename[] = {
	"../pic/black-1920-1080-32.bmp",
	"../pic/back-1920-1080-32.bmp"
};
const int WIDTH = 1920, HEIGHT = 1080, BITS = 32;
int filesize;

BYTE *readpic(const char *filename)
{
	HFILE pic = _lopen(filename, OF_READ);
	filesize = GetFileSize((HANDLE)pic, NULL);
	BYTE *buf = (BYTE*)malloc(filesize);
	_lread(pic, buf, filesize); 
	_lclose(pic);
	return buf;
}

void n_mmx(BYTE *p1, BYTE *p2, BYTE *ptar, int size, int fade)
{
	while(size--)
	{
		int res = (*p2) - (*p1);
		res = (res * fade) >> 8; 	// p2*fade/256 - p1*fade/256
		res += *p1;  				// p2*fade/256 + p1*(1-fade/256)
		*ptar = res;

		++p1, ++p2, ++ptar;
	}
}

void y_mmx(BYTE *pic1,BYTE *pic2,BYTE *pic,int size,int fade)
{
	size /= 4;

	__int32 *p1 = (__int32*)(pic1);
	__int32 *p2 = (__int32*)(pic2);
	__int32 *ptar = (__int32*)(pic);

	__int16 fade1[4] = {255-fade, 255-fade, 255-fade, 255-fade};
	__int16 fade2[4] = {fade, fade, fade, fade};
	_asm 
	{
		movq mm3, [fade1];					// 00RR 00RR 00RR 00RR
		movq mm4, [fade2];
	}
	/* 不能先减再乘除,可能产生负数 */
	for (unsigned int i = 0; i < size; ++i)
	{
	    __asm
	    {
			pxor 		mm0, mm0	  		// 清除mm0
	        //将所需数据移入寄存器
	        mov			esi, [p1]			
            mov			edx, [p2]			
            mov			edi, [ptar]
			movd 		mm1, [esi] 			// UUUU UUUU XXXX XXXX
			movd 		mm2, [edx]			// UUUU UUUU YYYY YYYY

			//将mm0和mm1解开，构成 00XX 00XX 00XX 00XX形式
			punpcklbw 	mm1, mm0			// 00XX 00XX 00XX 00XX
			punpcklbw 	mm2, mm0			// 00YY 00YY 00YY 00YY
			
			pmullw		mm1, mm3
			pmullw		mm2, mm4
			paddw		mm1, mm2			// (p1*(255-fade) + p2*fade) 
			psrlw		mm1, 8				// 00ZZ 00ZZ 00ZZ 00ZZ

			packuswb 	mm1, mm0 			// 0000 0000 ZZZZ ZZZZ
			//将结果传回目标位置
			movd		[edi], mm1
	    }
	    ++p1, ++p2, ++ptar;
	}
	_asm EMMS
}

int main(void)
{
	BYTE *buf1 = readpic(filename[0]), *buf2 = readpic(filename[1]);
	BYTE *buf = (BYTE*)malloc(filesize);
	HDC hdc = GetDC(NULL);
	BITMAPINFO *info = (BITMAPINFO*)(buf + 0x0e);

	for(int mmx=0;mmx<2;++mmx)
	{
		memcpy(buf,buf1,filesize);
		DWORD start_time =  GetTickCount();

		for(int step=-50;step<556;step+=2) //图一到图二,图二到图一
		{
			int fade;
			if(step<0) fade = 0;
			else if(step>=300) fade = 555 - step;
			else if(step>=256) fade = 256;
			else fade = step;

			if(mmx) y_mmx(buf1+0x36, buf2+0x36, buf+0x36, info->bmiHeader.biSizeImage, fade );
			else 	n_mmx(buf1+0x36, buf2+0x36, buf+0x36, info->bmiHeader.biSizeImage, fade );
			SetDIBitsToDevice(hdc,
				0, 0, WIDTH, HEIGHT, 	/* 目标区域左上角坐标,图片长宽 		*/
				0, 0, 0, HEIGHT, 		/* 图片左下角坐标,开始行数,有效行数 	*/
				buf + 0x36, info, DIB_RGB_COLORS
			);
		}

		DWORD end_time = GetTickCount();
		printf("%s method spend %d ms.\n",mmx?"MMX":"Normal",(end_time - start_time)); //输出运行时间
	}

	ReleaseDC(NULL, hdc);
	free(buf1), free(buf2), free(buf);
	return 0;
}
