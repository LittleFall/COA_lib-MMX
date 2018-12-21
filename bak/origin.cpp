#include <windows.h>
#include <stdio.h>

int Mmx(LPBYTE Picture1, LPBYTE Picture2, LPBYTE Picture, int intWidth, int intHeight, int RGB_Bit, int i);
int No_Mmx(LPBYTE Picture1, LPBYTE Picture2, LPBYTE Picture, int intWidth, int intHeight, int RGB_Bit, int i);
void Test_MMX1(int intWidth, int intHeight, int RGB_Bit, int Time);
void Test_MMX(int intWidth, int intHeight, int RGB_Bit, int Time);

int main(int argc, TCHAR* argv[])
{
	Test_MMX(640, 480, 24, 10);
	Test_MMX1(640, 480, 24, 10);

	system("pause");
	return 0;	
}

int Mmx(LPBYTE Picture1, LPBYTE Picture2, LPBYTE Picture, int intWidth, int intHeight, int RGB_Bit, int i)
{
	int x, y;
	LPDWORD temp1, temp2, temp;
	int fade_Rate = i * 128;
	//将fade值扩展为16位，以适应MMX的16位运算											
	WORD fade1[4], fade2[4];

	fade1[0] = fade1[1] = fade1[2] = fade1[3] = 32767 - fade_Rate;             //16位带符号数最大为32767
																			   //fade2[0] =fade2[1] =fade2[2] =fade2[3] = fade_Rate; 	

	_asm {
		movq   mm2, [fade1]
		//将阶值装入mm2寄存器
	}

	for (y = 0; y<intHeight; y++)
	{
		temp1 = (LPDWORD)(Picture1 + intWidth*RGB_Bit / 8 * y);
		temp2 = (LPDWORD)(Picture2 + intWidth*RGB_Bit / 8 * y);
		temp = (LPDWORD)(Picture + intWidth*RGB_Bit / 8 * y);

		x = 0;

		for (; x<intWidth; ++x, ++temp, ++temp1, ++temp2)
		{

			_asm {
				//Result_pixel = A_pixel*fade+B_pixel*(1-fade)=(A-B)*fade+B	
				pxor		mm7, mm7
				//将mm7寄存器清除为0		
				//movq		mm2,[fade1]			
				//movq		mm3,[fade2]			
				mov			esi, [temp1]
				mov			edx, [temp2]
				mov			edi, [temp]
				movd		mm0, [esi]
				//取图像1的像素分量装入mm0寄存器		
				movd		mm1, [edx]
				punpcklbw	mm0, mm7
				punpcklbw	mm1, mm7		//将字节解紧缩到16位	

				psubw       mm0, mm1     //首先做substraction: (A-B)*fade		
				pmulhw		mm0, mm2		//then the multiplication: (A-B)*fade		
											//	pmulhw		mm1,mm3	    
											paddw		mm0, mm1		//The last step: (A-B)*fade+B					
											packuswb	mm0, mm7	    //紧缩到16字节
											movd[edi], mm0
			}
		}
	}
	_asm EMMS
	// return 0;
}



int No_Mmx(LPBYTE Picture1, LPBYTE Picture2, LPBYTE Picture, int intWidth, int intHeight, int RGB_Bit, int i) 
{
	int x, y;

	LPBYTE temp1, temp, temp2;

	BYTE fade_Rate = (BYTE)i; //将fade值扩展为16位，以适应MMX的16位运算

	BYTE fade1, fade2;
	fade1 = 255 - fade_Rate;
	fade2 = fade_Rate;

	for (y = 0; y < intHeight; y++)
	{
		temp1 = (LPBYTE)(Picture1 + intWidth*RGB_Bit / 8 * y);
		temp2 = (LPBYTE)(Picture2 + intWidth*RGB_Bit / 8 * y);
		temp = (LPBYTE)(Picture + intWidth * RGB_Bit / 8 * y);

		for (x = 0; x < 3 * intWidth; x++) {
			_asm {
				mov dh, [fade1]
				mov dl, [fade2]
				mov ecx, [temp1]
				mov bh, [ecx]
				mov esi, [temp2]
				mov bl, [esi]
				mov al, dh
				mul bh
				mov edi, [temp]
				mov[edi], ah
				mov al, dl
				mul bl
				add ah, [edi]
				mov[edi], ah
			}
			temp1++;
			temp2++;
			temp++;
		}
	}
	_asm EMMS
	return 0;
}

void Test_MMX(int intWidth, int intHeight, int RGB_Bit, int Time)
{
	_try{
		_asm EMMS
	}
		_except(EXCEPTION_EXECUTE_HANDLER) {
	}
	
	LPBITMAPINFO lpInfo;
	int intSize, i;
	LPBYTE  lpBuf1, lpBuf2, lpBuf;
	HFILE Picture1, Picture2;
	Picture1 = _lopen("1.bmp", OF_READ);          //返回打开文件的句柄
	Picture2 = _lopen("2.bmp", OF_READ);
	intSize = GetFileSize((HANDLE)Picture1, NULL);//返回文件长度		
	lpBuf1 = (LPBYTE)LocalAlloc(LPTR, intSize);//返回新分配的内存地址，从堆中分配指定大小的字节数
	lpBuf2 = (LPBYTE)LocalAlloc(LPTR, intSize);
	lpBuf = (LPBYTE)LocalAlloc(LPTR, intSize);

	_lread(Picture1, lpBuf1, intSize);   //指定一个内存块的指针，数据将读入这个内存块
	_lread(Picture2, lpBuf2, intSize);
	_lclose(Picture1);
	_lclose(Picture2);
	memcpy(lpBuf, lpBuf1, intSize);
	lpInfo = (LPBITMAPINFO)(lpBuf + 0x0e);
	HDC hDC = GetDC(NULL);  //指向图片对象的长指针类型

	DWORD   start_time = GetTickCount();
	for (i = 0; i<255; i++)    //默认是图像按255阶渐变，即让fade的值变化255次		
	{
		Mmx(lpBuf1 + 0x60, lpBuf2 + 0x60, lpBuf + 0x60, intWidth, intHeight, RGB_Bit, i);
		Sleep(Time);    //指定从lpBuf+0x60这一行开始扫描				
		SetDIBitsToDevice(hDC, 300, 100, intWidth, intHeight, 0, 0, 0, 480, lpBuf + 0x60, lpInfo, DIB_RGB_COLORS);
	}

	DWORD end_time = GetTickCount();
	printf("MMX method: %d ms.\n", (end_time - start_time)); //输出运行时间

	LocalFree(lpBuf1);
	LocalFree(lpBuf2);
	ReleaseDC(NULL, hDC);
}

void Test_MMX1(int intWidth, int intHeight, int RGB_Bit, int Time) 
{
	_try{
		_asm EMMS
	}
		_except(EXCEPTION_EXECUTE_HANDLER) {

	}
	HDC hDC = GetDC(NULL);  //指向图片对象的长指针类型
	HFILE Picture1 = _lopen("1.bmp", OF_READ); //返回打开文件的句柄
	HFILE Picture2 = _lopen("2.bmp", OF_READ);

	int intSize = GetFileSize((HANDLE)Picture1, NULL); //返回文件长度

	LPBYTE lpBuf1 = (LPBYTE)LocalAlloc(LPTR, intSize); //返回新分配的内存地址，从堆中分配指定大小的字节数

	LPBYTE lpBuf2 = (LPBYTE)LocalAlloc(LPTR, intSize);

	LPBYTE lpBuf = (LPBYTE)LocalAlloc(LPTR, intSize);

	_lread(Picture1, lpBuf1, intSize);  //指定一个内存块的指针，将数据读入这个内存块
	_lread(Picture2, lpBuf2, intSize);
	_lclose(Picture1);
	_lclose(Picture2);
	memcpy(lpBuf, lpBuf1, intSize);
	LPBITMAPINFO lpInfo = (LPBITMAPINFO)(lpBuf + 0x0e);

	DWORD start_time = GetTickCount();

	for (int i = 0; i < 255; i++)
	{
		//默认是图像按255阶渐变，即让fade的值变化255次
		No_Mmx(lpBuf1 + 0x60, lpBuf2 + 0x60, lpBuf + 0x60, intWidth, intHeight, RGB_Bit, i);
		Sleep(Time);
		//指定从lpBuf+0x60这一行开始扫描
		SetDIBitsToDevice(hDC, 300, 150, intWidth, intHeight, 0, 0, 0, 480, lpBuf + 0x60, lpInfo, DIB_RGB_COLORS);
		//该函数使用DIB位图和颜色数据对与目标设备环境相关的设备上的指定矩形中的像素进行设置。
	}

	DWORD end_time = GetTickCount();

	printf("Normal method: %d ms.\n", (end_time - start_time)); //输出运行时间

	LocalFree(lpBuf1);
	LocalFree(lpBuf2);
	ReleaseDC(NULL, hDC);
}
