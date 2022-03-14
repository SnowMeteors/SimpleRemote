#include <atlimage.h>
extern "C" BOOL ScreenShot(char path[]);

// 屏幕截图 图片保存到path路径下
BOOL ScreenShot(char path[])
{
	int ret;
	// char * 转 LPCTSTR
	CString save = path;
	// 获取桌面窗口句柄
	HDC hdcSrc = GetDC(NULL);
	if (hdcSrc == NULL)
		return FALSE;

	int nBitPerPixel = GetDeviceCaps(hdcSrc, BITSPIXEL);
	int nWidth = GetDeviceCaps(hdcSrc, HORZRES);
	int nHeight = GetDeviceCaps(hdcSrc, VERTRES);
	CImage image;
	ret = image.Create(nWidth, nHeight, nBitPerPixel);
	if (!ret)
		return FALSE;

	ret = BitBlt(image.GetDC(), 0, 0, nWidth, nHeight, hdcSrc, 0, 0, SRCCOPY);
	if (!ret)
		return FALSE;

	ret = ReleaseDC(NULL, hdcSrc);
	if (!ret)
		return FALSE;

	image.ReleaseDC();
	image.Save((LPCTSTR)save, Gdiplus::ImageFormatPNG);//ImageFormatJPEG

	return TRUE;
}
