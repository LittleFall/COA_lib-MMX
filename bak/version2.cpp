#include <windows.h>
#include <stdio.h>

int Mmx(LPBYTE Picture1, LPBYTE Picture2, LPBYTE Picture, int intWidth, int intHeight, int RGB_Bit, int i);
int No_Mmx(LPBYTE Picture1, LPBYTE Picture2, LPBYTE Picture, int intWidth, int intHeight, int RGB_Bit, int i);
void Test_MMX(int intWidth, int intHeight, int RGB_Bit, int Time,int tag);

const int WIDTH = 1920, HEIGHT = 1080, RGB_BITS = 24;
const char* picFileName[2]={"../pic/white-1920-1080-24.bmp","../pic/black-1920-1080-24.bmp"};
int main(int argc, TCHAR *argv[])
{
    Test_MMX(WIDTH, HEIGHT, RGB_BITS, 0, 0);
    //Test_MMX(WIDTH, HEIGHT, RGB_BITS, 0, 1);

    return 0;
}

void Test_MMX(int intWidth, int intHeight, int RGB_Bit, int Time, int isMMX)
{
    HFILE Picture1 = _lopen(picFileName[0], OF_READ);          //返回打开文件的句柄
    HFILE Picture2 = _lopen(picFileName[1], OF_READ);
    int intSize = GetFileSize((HANDLE)Picture1, NULL); //返回文件长度

    LPBYTE lpBuf1 = (LPBYTE)LocalAlloc(LPTR, intSize); //返回新分配的内存地址，从堆中分配指定大小的字节数
    LPBYTE lpBuf2 = (LPBYTE)LocalAlloc(LPTR, intSize);
    LPBYTE lpBuf = (LPBYTE)LocalAlloc(LPTR, intSize);

    _lread(Picture1, lpBuf1, intSize);   //指定一个内存块的指针	，数据将读入这个内存块
    _lread(Picture2, lpBuf2, intSize);
    memcpy(lpBuf, lpBuf1, intSize);

    _lclose(Picture1);
    _lclose(Picture2);

    LPBITMAPINFO lpInfo = (LPBITMAPINFO)(lpBuf + 0x0e);
    DWORD start_time =  GetTickCount();

    HDC hDC = GetDC(NULL);  //指向图片对象的长指针类型
    for (int i = 0; i < 256; i++)  //默认是图像按255阶渐变，即让fade的值变化255次
    {
    	if(isMMX)
       		   Mmx(lpBuf1 + 0x36, lpBuf2 + 0x36, lpBuf + 0x36, intWidth, intHeight, RGB_Bit, i);
    	else
    		No_Mmx(lpBuf1 + 0x36, lpBuf2 + 0x36, lpBuf + 0x36, intWidth, intHeight, RGB_Bit, i);
        SetDIBitsToDevice(hDC, 0, 0, intWidth, intHeight, 0, 0, 0, intHeight, lpBuf + 0x36, lpInfo, DIB_RGB_COLORS);
    }
    ReleaseDC(NULL, hDC);

    DWORD end_time = GetTickCount();
    printf("%s method: %d ms.\n",isMMX?"MMX":"NoMMX",(end_time - start_time)); //输出运行时间

    LocalFree(lpBuf1);
    LocalFree(lpBuf2);
    LocalFree(lpBuf);
}

int Mmx(LPBYTE Picture1, LPBYTE Picture2, LPBYTE Picture, int intWidth, int intHeight, int RGB_Bit, int fade_Rate)
{
    fade_Rate *= 128; //将fade值扩展为16位，以适应MMX的16位运算
    WORD fade1[4];

    fade1[0] = fade1[1] = fade1[2] = fade1[3] = 32767 - fade_Rate;             //16位带符号数最大为32767
    
    _asm
    {
        movq   mm2, [fade1]
        //将阶值装入mm2寄存器
    }

    for (int y = 0; y<intHeight; y++)
    {
        LPDWORD temp1 = (LPDWORD)(Picture1 + intWidth * RGB_Bit / 8 * y);
        LPDWORD temp2 = (LPDWORD)(Picture2 + intWidth * RGB_Bit / 8 * y);
        LPDWORD temp = (LPDWORD)(Picture + intWidth * RGB_Bit / 8 * y);

        for (int x = 0; x < intWidth; ++x, ++temp, ++temp1, ++temp2)
        {
            _asm
            {
                pxor		mm7, mm7
                //将mm7寄存器清除为0
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
    return 0;
}



int No_Mmx(LPBYTE Picture1, LPBYTE Picture2, LPBYTE Picture, int intWidth, int intHeight, int RGB_Bit, int rate)
{
    BYTE fade_Rate = (BYTE)rate; 
    BYTE fade1 = 255 - fade_Rate;
    BYTE fade2 = fade_Rate;

    for (int y = 0; y < intHeight; y++)
    {
        LPBYTE temp1 = (LPBYTE)(Picture1 + intWidth * RGB_Bit / 8 * y);
        LPBYTE temp2 = (LPBYTE)(Picture2 + intWidth * RGB_Bit / 8 * y);
        LPBYTE temp = (LPBYTE)(Picture + intWidth * RGB_Bit / 8 * y);

        for (int x = 0; x < 3 * intWidth; x++)
        {
            _asm
            {
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
    return 0;
}
