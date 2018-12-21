void CTestDlg::OnButton1() 
{
	//读入位图文件名
	char *filename = "720bmp16.bmp";
	CDC *pDC = GetDC();
 
	//
	// 一次性将位图文件读入到内容中，待处理
	//
    CFile f;
	f.Open(filename, CFile::modeRead);
    char* buff = (char*)malloc(f.GetLength());
	f.SeekToBegin();
    f.Read(buff, f.GetLength());
    
	//
    BITMAPFILEHEADER	*fileheader = NULL; //数据结构大小为14
    BITMAPINFO			*info = NULL;		//数据结构大小为40
 
    fileheader = (BITMAPFILEHEADER*)buff;
 
    if(fileheader->bfType!=0x4D42)
    {
        AfxMessageBox("不是BMP文件");
        f.Close();
        return ;
    }
 
	//位图信息区域，在偏移14处
    info = (BITMAPINFO*)(buff+(sizeof(BITMAPFILEHEADER)));
 
 
	//位图的宽和高
    int width = info->bmiHeader.biWidth;
    int height= info->bmiHeader.biHeight;
 
	//指向位图像素区域
	char* buffer = buff+fileheader->bfOffBits;
 
	//
	//	显示位图的一些基本信息
	//
	CString str;
	str.Format("位图大小= %d\n"
				"位图数据起始偏移 = %d\n"
				"BITMAPINFOHEADER.biSize = %d\n"
				"宽=%d, 高=%d\n"
				"颜色位数=%d\n"
				"压缩=%d\n"
				"biSizeImage=%d\n"
				"biClrUsed=%d\n"
				"biClrImportant=%d",
 
 
				fileheader->bfSize,		//位图大小
				fileheader->bfOffBits,	//位图数据起始偏移
				info->bmiHeader.biSize,
				info->bmiHeader.biWidth,
				info->bmiHeader.biHeight,
				info->bmiHeader.biBitCount,
				info->bmiHeader.biCompression,
				info->bmiHeader.biSizeImage,
				info->bmiHeader.biClrUsed,
				info->bmiHeader.biClrImportant
 
		);
	AfxMessageBox(str);
 
    int i,j;
	//
    // 单色图的解析
	//

    if(info->bmiHeader.biBitCount==32)
    {
        // bgra
        if(height>0)
        {
            //height>0 表示图片颠倒
            for(i=0;i<height;i++)
            {
                for(j=0;j<width;j++)
                {
                    unsigned char b=buffer[(i*width+j)*4];
                    unsigned char g=buffer[(i*width+j)*4+1];
                    unsigned char r=buffer[(i*width+j)*4+2];
                    pDC->SetPixel(j,height-i, RGB(r,g,b));
                }
            }
 
 
        }
        else
        {
            for(i=0;i<0-height;i++)
            {
                for(j=0;j<width;j++)
                {
                    unsigned char b=buffer[(i*width+j)*4];
                    unsigned char g=buffer[(i*width+j)*4+1];
                    unsigned char r=buffer[(i*width+j)*4+2];
                    pDC->SetPixel(j, i, RGB(r,g,b));
                }
            }
        }
    }
 
    f.Close();
    buff = NULL;
    free(buff);
	ReleaseDC(pDC);
}
