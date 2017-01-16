#include <windows.h>
#include <gdiplus.h>
#include <stdio.h>
#include <string.h>
#include <io.h>
#include <time.h>
#include "data.h"
//#include "resource.h"

#pragma warning(disable:4996)
using namespace Gdiplus;
#pragma comment(lib, "gdiplus")

const int WIDTH = 800;
const int HEIGHT = 600;

const int CLASS_NUM = 2;
const int DATA_NUM = 500;
const int TEST_NUM = 100;
const int M = CLASS_NUM*DATA_NUM;
const long double E = 2.718281828;

HINSTANCE g_hInst;
HWND hWndMain;
LPCTSTR lpszClass = TEXT("GdiPlusStart");


const int w_max=12, h_max=14;
const int N = w_max * h_max;
const int Kernel_Mode = 0;

long double W[N];
Data X[M];
long double kernel_matrix[M][M];
Data T_X[CLASS_NUM*TEST_NUM];
int Y[M];
int T_Y[CLASS_NUM*TEST_NUM];
long double Alpha[M];
long double C = 1000.0;
long double Bias = 0.0;

void OnPaint(HDC hdc, int ID, int x, int y);
void OnPaintA(HDC hdc, int ID, int x, int y, double alpha);
Color GetPixel(HDC hdc, int ID, int x, int y);
bool CompareClr(Color A, Color B);
void ReadAndDraw(HDC hdc, WCHAR* filename);
void PreProc(HDC hdc, WCHAR* filename, int first, int second, int num);
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);
void Img2Txt(WCHAR* filename, WCHAR* txt_filename, int h_max, int w_max);
void ReadTrainingData(WCHAR* filename, int ith, int h_max, int w_max);
void ReadTestData(WCHAR* filename, int ith, int h_max, int w_max);
long double kernel_func(Data x1, Data x2);
long double predict(Data x);
long double sigmoid(long double x);

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{
	HWND     hWnd;
	MSG		 msg;
	WNDCLASS WndClass;

	g_hInst = hInstance;

	ULONG_PTR gpToken;
	GdiplusStartupInput gpsi;
	if (GdiplusStartup(&gpToken, &gpsi, NULL) != Ok)
	{
		MessageBox(NULL, TEXT("GDI+ 라이브러리를 초기화할 수 없습니다."), TEXT("알림"), MB_OK);
		return 0;
	}


	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc = WndProc;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hInstance = hInstance;
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	WndClass.lpszMenuName = NULL;
	WndClass.lpszClassName = L"Window Class Name";
	RegisterClass(&WndClass);
	hWnd = CreateWindow(
		L"Window Class Name",
		L"Window Title Name",
		WS_OVERLAPPEDWINDOW,
		GetSystemMetrics(SM_CXFULLSCREEN) / 2 - WIDTH/2,
		GetSystemMetrics(SM_CYFULLSCREEN) / 2 - HEIGHT/2,
		WIDTH,
		HEIGHT,
		NULL,
		NULL,
		hInstance,
		NULL
		);
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	HDC hdc, MemDC;
	PAINTSTRUCT ps;

	HBITMAP hBit, OldBit;
	RECT crt;

	static int loop = 0;
	static int num = 0;
	static WCHAR filename[1024],txt_filename[1024];

	switch (iMsg)
	{
	case WM_CREATE:
		//SetTimer(hWnd, 1, 10, 0);
		//InvalidateRect(hWnd, NULL, FALSE);

		srand((unsigned)time(NULL));
		
		//initialize
		for(int i = 0; i < M; i++)
		{
			X[i] = Data(h_max, w_max);
		}
		for(int i = 0; i < CLASS_NUM*TEST_NUM; i++)
		{
			T_X[i] = Data(h_max, w_max);
		}

		//read data 
		for(int i = 0; i < CLASS_NUM; i++)
		{
			printf("%d\n",i);
			fflush(stdout);
			for(int j = 0; j < DATA_NUM; j++)
			{
				if(j%500==0)
				{
					printf("  %d\n",j);
					fflush(stdout);
				}
				//wsprintf(filename,L"//vmware-host/Shared Folders/Downloads/classified/sorted/preproc/%d/%d.png",i,j);
				wsprintf(txt_filename,L"//vmware-host/Shared Folders/Downloads/classified/sorted/preproc/%d/%d.txt",i,j);
				//Img2Txt(filename, txt_filename, h_max, w_max);				
				ReadTrainingData(txt_filename, i*(DATA_NUM)+j, h_max, w_max);
				if(i==0)
					Y[i*(DATA_NUM)+j] = 1;
				else
					Y[i*(DATA_NUM)+j] = -1;
			}
		}

		printf("input trainig done\n");
		fflush(stdout);

		for(int i = 0; i < CLASS_NUM; i++)
		{
			printf("%d\n",i);
			fflush(stdout);
			for(int j = 0; j < TEST_NUM; j++)
			{
				if(j%50==0)
				{
					printf("  %d\n",j);
					fflush(stdout);
				}
				
				wsprintf(txt_filename,L"//vmware-host/Shared Folders/Downloads/classified/sorted/preproc/%d/%d.txt",i,j+DATA_NUM);
				ReadTestData(txt_filename, i*(TEST_NUM)+j, h_max, w_max);
				
				if(i==0)
					T_Y[i*(TEST_NUM)+j] = 1;
				else
					T_Y[i*(TEST_NUM)+j] = -1;
			}
		}

		printf("input test done\n");
		fflush(stdout);

		for(int i=0; i<M; i++)
		{
			if(i%50==0)
			{
				printf("%d\n",i);
				fflush(stdout);
			}
			for(int j=0; j<M; j++)
			{
				kernel_matrix[i][j] = kernel_func(X[i],X[j]);
			}
		}

		printf("kernel matrix done\n");
		fflush(stdout);

		while(1)
		{
			/*int one = rand()%M;
			int two = rand()%(M-1);
			if(one == two)
				two = M-1;*/
			int one = -1, two = -1;

			long double max_one = 0, max_two = 0;
			for(int i = 0; i<M; i++)
			{
				long double ret = 1 - Y[i]*(predict(X[i])-Bias) - 2*Alpha[i]*kernel_matrix[i][i];
				if (ret >= max_one)
				{
					max_two = max_one;
					two = one;
					max_one = ret;
					one = i;
				}
			}
			

			//printf("%dth    %d %d\n",loop,one,two);
			//fflush(stdout);

			long double U,V;
			if (Y[one] != Y[two])
			{
				U = max(0, Alpha[two] - Alpha[one]);
				V = min(C, C - Alpha[one] + Alpha[two]);
			}
			else
			{
				U = max(0, Alpha[one] + Alpha[two] - C);
				V = min(C, Alpha[one] + Alpha[two]);
			}

			long double E1 = predict(X[one]) - Y[one];
			long double E2 = predict(X[two]) - Y[two];
			long double K = kernel_matrix[one][one] + kernel_matrix[two][two] - 2 * kernel_matrix[one][two];
			long double Alpha_two_new_unc = Alpha[two] + (Y[two] * (E1-E2) / K);
			long double Alpha_two_new;

			//printf("E1:%Lf, E2:%Lf, K:%Lf, Alpha_two_new_unc = %Lf\n",E1,E2,K,Alpha_two_new_unc);
			//fflush(stdout);

			if (Alpha_two_new_unc > V)
				Alpha_two_new = V;
			else if (Alpha_two_new_unc < U)
				Alpha_two_new = U;
			else
				Alpha_two_new = Alpha_two_new_unc;

			long double Alpha_one_new = Alpha[one] + Y[one]*Y[two]*(Alpha[two] - Alpha_two_new);

			//get W
			if (Kernel_Mode == 0)
			{
				for(int i=0; i<N; i++)
				{
					W[i] -= Alpha[one]*Y[one]*X[one].d1(i);
					W[i] -= Alpha[two]*Y[two]*X[two].d1(i);

					W[i] += Alpha_one_new * Y[one] * X[one].d1(i);
					W[i] += Alpha_two_new * Y[two] * X[two].d1(i);
				}
			}

			Alpha[one] = Alpha_one_new;
			Alpha[two] = Alpha_two_new;

			loop++;






			if(loop%5000==1)
			{
				printf("one: %d, two:%d    max_one:%Lf, max_two:%Lf\n",one,two,max_one,max_two);
				fflush(stdout);

				long double L = 0;
				for(int i = 0; i < M; i++)
				{
					L += Alpha[i];
				}
				long double ret = 0;

				if (Kernel_Mode == 0)
				{
					for (int i = 0; i<N; i++)
						ret += W[i]*W[i];
				}
				else
				{
					for(int i = 0; i < M; i++)
					{
						for(int j = 0; j < M; j++)
						{
							ret += Alpha[i] * Alpha[j] * Y[i] * Y[j] * kernel_matrix[i][j];
						}
					}
				}
				L -= 0.5*ret;





				int wrong_count = 0;
				long double error_sum = 0;
				for(int i = 0; i<M; i++)
				{
					/*if(i%500==0)
					{
						printf("22222 %d\n",i);
						fflush(stdout);
					}*/
					long double predicted = sigmoid(predict(X[i]))*2-1;
					if((Y[i]==1 && predicted < 0.5) || (Y[i]==-1 && predicted > -0.5))
						wrong_count++;
					error_sum += abs(predicted - Y[i]);
					//printf("predicted: %9.4Lf -> %9.4Lf   real: %d\n",predict(X[i]),sigmoid(predict(X[i]))*2-1,Y[i]);
					//fflush(stdout);
				}

				int test_wrong_count = 0;
				long double test_error_sum = 0;
				for(int i = 0; i<CLASS_NUM*TEST_NUM; i++)
				{
					/*if(i%500==0)
					{
						printf("22222 %d\n",i);
						fflush(stdout);
					}*/
					long double predicted = sigmoid(predict(T_X[i]))*2-1;
					if((T_Y[i]==1 && predicted < 0.5) || (T_Y[i]==-1 && predicted > -0.5))
						test_wrong_count++;
					test_error_sum += abs(predicted - T_Y[i]);
					//printf("predicted: %9.4Lf -> %9.4Lf   real: %d\n",predict(X[i]),sigmoid(predict(X[i]))*2-1,Y[i]);
					//fflush(stdout);
				}

				//printf("L: %Lf    ret:%Lf    alpha1:%Lf, alpha2:%Lf\n\n",L,ret,Alpha[one],Alpha[two]);
				printf("L: %Lf   wrong:%d/%d(=%Lf)  error_sum(cost):%Lf    error_avg:%Lf    /test:/  wrong:%d/%d(=%Lf)  error_sum(cost):%Lf    error_avg:%Lf\n",
					L,wrong_count,M,(long double)wrong_count/M, error_sum, error_sum/(long double)M,
					test_wrong_count,CLASS_NUM*TEST_NUM,(long double)test_wrong_count/(CLASS_NUM*TEST_NUM), test_error_sum, test_error_sum/(long double)(CLASS_NUM*TEST_NUM));
				fflush(stdout);
			}
		}

		for(int i = 0; i < M; i++)
		{
			long double ret = (1 - Y[i]*(predict(X[i])-Bias))/Y[i];
			printf("%Lf\n",ret);
			fflush(stdout);
			Bias = ret;
		}
		printf("\n");


		break;

	case WM_TIMER:
		//InvalidateRect(hWnd, NULL, FALSE);
		//break;

	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		GetClientRect(hWnd, &crt);

		MemDC = CreateCompatibleDC(hdc);
		hBit = CreateCompatibleBitmap(hdc, crt.right, crt.bottom);
		OldBit = (HBITMAP)SelectObject(MemDC, hBit);
		//hBrush = CreateSolidBrush(RGB(255, 255, 255));
		//oldBrush = (HBRUSH)SelectObject(MemDC, hBrush);
		//hPen = CreatePen(PS_SOLID, 5, RGB(255, 255, 255));
		//oldPen = (HPEN)SelectObject(MemDC, hPen);

		//FillRect(MemDC, &crt, hBrush);
		//SetBkColor(MemDC, RGB(255, 255, 255));
		
		

		BitBlt(hdc, 0, 0, crt.right, crt.bottom, MemDC, 0, 0, SRCCOPY);
		SelectObject(MemDC, OldBit);
		DeleteDC(MemDC);
		//SelectObject(MemDC, oldPen);
		//DeleteObject(hPen);
		//SelectObject(MemDC, oldBrush);
		//DeleteObject(hBrush);
		DeleteObject(hBit);

		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hWnd, iMsg, wParam, lParam);
}

void OnPaint(HDC hdc, int ID, int x, int y)
{
	Graphics G(hdc);
	HRSRC hResource = FindResource(g_hInst, MAKEINTRESOURCE(ID), TEXT("PNG"));
	if (!hResource)
		return;

	DWORD imageSize = SizeofResource(g_hInst, hResource);
	HGLOBAL hGlobal = LoadResource(g_hInst, hResource);
	LPVOID pData = LockResource(hGlobal);

	HGLOBAL hBuffer = GlobalAlloc(GMEM_MOVEABLE, imageSize);
	LPVOID pBuffer = GlobalLock(hBuffer);
	CopyMemory(pBuffer, pData, imageSize);
	GlobalUnlock(hBuffer);

	IStream *pStream;
	HRESULT hr = CreateStreamOnHGlobal(hBuffer, TRUE, &pStream);

	Image I(pStream);
	pStream->Release();
	if (I.GetLastStatus() != Ok) return;

	G.DrawImage(&I, x, y, I.GetWidth(), I.GetHeight());
}

void OnPaintA(HDC hdc, int ID, int x, int y, double alpha)
{
	Graphics G(hdc);
	HRSRC hResource = FindResource(g_hInst, MAKEINTRESOURCE(ID), TEXT("PNG"));
	if (!hResource)
		return;

	ColorMatrix ClrMatrix =
	{
		1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, (Gdiplus::REAL)alpha, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f, 1.0f
	};

	ImageAttributes ImgAttr;
	ImgAttr.SetColorMatrix(&ClrMatrix, ColorMatrixFlagsDefault, ColorAdjustTypeBitmap);

	DWORD imageSize = SizeofResource(g_hInst, hResource);
	HGLOBAL hGlobal = LoadResource(g_hInst, hResource);
	LPVOID pData = LockResource(hGlobal);

	HGLOBAL hBuffer = GlobalAlloc(GMEM_MOVEABLE, imageSize);
	LPVOID pBuffer = GlobalLock(hBuffer);
	CopyMemory(pBuffer, pData, imageSize);
	GlobalUnlock(hBuffer);

	IStream *pStream;
	HRESULT hr = CreateStreamOnHGlobal(hBuffer, TRUE, &pStream);

	Image I(pStream);
	pStream->Release();
	if (I.GetLastStatus() != Ok) return;

	RectF destination(0, 0, (Gdiplus::REAL)I.GetWidth(), (Gdiplus::REAL)I.GetHeight());
	G.DrawImage(&I, destination, x, y, (Gdiplus::REAL)I.GetWidth(), (Gdiplus::REAL)I.GetHeight(), UnitPixel, &ImgAttr);
}

Color GetPixel(HDC hdc, int ID, int x, int y)
{
	HRSRC hResource = FindResource(g_hInst, MAKEINTRESOURCE(ID), TEXT("PNG"));
	if (!hResource)
		return NULL;

	DWORD imageSize = SizeofResource(g_hInst, hResource);
	HGLOBAL hGlobal = LoadResource(g_hInst, hResource);
	LPVOID pData = LockResource(hGlobal);

	HGLOBAL hBuffer = GlobalAlloc(GMEM_MOVEABLE, imageSize);
	LPVOID pBuffer = GlobalLock(hBuffer);
	CopyMemory(pBuffer, pData, imageSize);
	GlobalUnlock(hBuffer);

	IStream *pStream;
	HRESULT hr = CreateStreamOnHGlobal(hBuffer, TRUE, &pStream);

	Bitmap bitmap(pStream, true);
	pStream->Release();

	Color clr;
	bitmap.GetPixel(x,y,&clr);
	return clr;
}

void ReadAndDraw(HDC hdc, WCHAR* filename)
{
	Graphics G(hdc);

	Bitmap bitmap(filename, true);
	
	
	ColorMatrix ClrMatrix =
	{
		1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, (Gdiplus::REAL)1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f, 1.0f
	};
	ImageAttributes ImgAttr;
	ImgAttr.SetColorMatrix(&ClrMatrix, ColorMatrixFlagsDefault, ColorAdjustTypeBitmap);
	RectF destination(0, 0, (Gdiplus::REAL)bitmap.GetWidth(), (Gdiplus::REAL)bitmap.GetHeight());
	G.DrawImage(&bitmap, destination, 0, 0, (Gdiplus::REAL)bitmap.GetWidth(), (Gdiplus::REAL)bitmap.GetHeight(), UnitPixel, &ImgAttr);
}

bool CompareClr(Color A, Color B)
{
	if(A.GetR() == B.GetR() && A.GetG() == B.GetG() && A.GetB() == B.GetB())
		return true;
	return false;
}

//code from MSDN(https://msdn.microsoft.com/en-us/library/windows/desktop/ms533843(v=vs.85).aspx)
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
   UINT  num = 0;          // number of image encoders
   UINT  size = 0;         // size of the image encoder array in bytes

   ImageCodecInfo* pImageCodecInfo = NULL;

   GetImageEncodersSize(&num, &size);
   if(size == 0)
      return -1;  // Failure

   pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
   if(pImageCodecInfo == NULL)
      return -1;  // Failure

   GetImageEncoders(num, size, pImageCodecInfo);

   for(UINT j = 0; j < num; ++j)
   {
      if( wcscmp(pImageCodecInfo[j].MimeType, format) == 0 )
      {
         *pClsid = pImageCodecInfo[j].Clsid;
         free(pImageCodecInfo);
         return j;  // Success
      }    
   }

   free(pImageCodecInfo);
   return -1;  // Failure
}
void PreProc(HDC hdc, WCHAR* filename, int first, int second, int num)
{
	WCHAR new_filename[1024];
	CLSID pngClsid;
	Graphics G(hdc);

	Bitmap bmp1(filename, true);
	Bitmap bmp2(filename, true);

	Color bg(192,192,192);
	Color color;

	RECT rect1, rect2;

	for(int i=0; i<(int)bmp1.GetWidth(); i++)
	{
		int j;
		for(j=0; j<(int)bmp1.GetHeight(); j++)
		{
			Color clr;
			bmp1.GetPixel(i,j,&clr);
			if(!CompareClr(clr, bg))
			{
				color = clr;
				break;
			}
		}
		if(j!=(int)bmp1.GetHeight())
			break;
	}

	for(int i=0; i<(int)bmp1.GetHeight(); i++)
	{
		for(int j=0; j<(int)bmp1.GetWidth(); j++)
		{
			Color clr;
			bmp1.GetPixel(j,i,&clr);
			if(!CompareClr(clr, bg) && !CompareClr(clr, color))
			{
				bmp1.SetPixel(j,i,bg);		
			}
		}
	}

	////////////////////////////////////////////////////////////////////////////
	for(int i=0; i<(int)bmp1.GetWidth(); i++)
	{
		int j;
		for(j=0; j<(int)bmp1.GetHeight(); j++)
		{
			Color clr;
			bmp1.GetPixel(i,j,&clr);
			if(!CompareClr(clr, bg))
			{
				rect1.left = i;
				break;
			}
		}
		if(j!=(int)bmp1.GetHeight())
			break;
	}
	for(int i=(int)bmp1.GetWidth()-1; i>=0; i--)
	{
		int j;
		for(j=0; j<(int)bmp1.GetHeight(); j++)
		{
			Color clr;
			bmp1.GetPixel(i,j,&clr);
			if(!CompareClr(clr, bg))
			{
				rect1.right = i;
				break;
			}
		}
		if(j!=(int)bmp1.GetHeight())
			break;
	}
	for(int i=0; i<(int)bmp1.GetHeight(); i++)
	{
		int j;
		for(j=0; j<(int)bmp1.GetWidth(); j++)
		{
			Color clr;
			bmp1.GetPixel(j,i,&clr);
			if(!CompareClr(clr, bg))
			{
				rect1.top = i;
				break;
			}
		}
		if(j!=(int)bmp1.GetWidth())
			break;
	}
	for(int i=(int)bmp1.GetHeight()-1; i>=0; i--)
	{
		int j;
		for(j=0; j<(int)bmp1.GetWidth(); j++)
		{
			Color clr;
			bmp1.GetPixel(j,i,&clr);
			if(!CompareClr(clr, bg))
			{
				rect1.bottom = i;
				break;
			}
		}
		if(j!=(int)bmp1.GetWidth())
			break;
	}
	////////////////////////////////////////////////////////////////////////////

	Bitmap n_bmp1(rect1.right-rect1.left+1, rect1.bottom-rect1.top+1, bmp1.GetPixelFormat());

	for(int i = 0; i < n_bmp1.GetHeight(); i++)
	{
		for(int j = 0; j < n_bmp1.GetWidth(); j++)
		{
			Color clr;
			bmp1.GetPixel(j+rect1.left, i+rect1.top, &clr);
			n_bmp1.SetPixel(j,i,clr);
		}
	}

	wsprintf(new_filename,L"//vmware-host/Shared Folders/Downloads/classified/sorted/preproc/%d_%d.png",first,num*2);
	GetEncoderClsid(L"image/png", &pngClsid);
	n_bmp1.Save(new_filename, &pngClsid, NULL);
	
	/*ColorMatrix ClrMatrix =
	{
		1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, (Gdiplus::REAL)1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f, 1.0f
	};
	ImageAttributes ImgAttr;
	ImgAttr.SetColorMatrix(&ClrMatrix, ColorMatrixFlagsDefault, ColorAdjustTypeBitmap);
	RectF destination1(0, 0, (Gdiplus::REAL)n_bmp1.GetWidth(), (Gdiplus::REAL)n_bmp1.GetHeight());
	G.DrawImage(&n_bmp1, destination1, 0, 0, (Gdiplus::REAL)n_bmp1.GetWidth(), (Gdiplus::REAL)n_bmp1.GetHeight(), UnitPixel, &ImgAttr);*/


	for(int i=(int)bmp2.GetWidth()-1; i>=0; i--)
	{
		int j;
		for(j=0; j<(int)bmp2.GetHeight(); j++)
		{
			Color clr;
			bmp2.GetPixel(i,j,&clr);
			if(!CompareClr(clr, bg))
			{
				color = clr;
				break;
			}
		}
		if(j!=(int)bmp2.GetHeight())
			break;
	}

	for(int i=0; i<(int)bmp2.GetHeight(); i++)
	{
		for(int j=0; j<(int)bmp2.GetWidth(); j++)
		{
			Color clr;
			bmp2.GetPixel(j,i,&clr);
			if(!CompareClr(clr, bg) && !CompareClr(clr, color))
			{
				bmp2.SetPixel(j,i,bg);		
			}
		}
	}

	////////////////////////////////////////////////////////////////////////////
	for(int i=0; i<(int)bmp2.GetWidth(); i++)
	{
		int j;
		for(j=0; j<(int)bmp2.GetHeight(); j++)
		{
			Color clr;
			bmp2.GetPixel(i,j,&clr);
			if(!CompareClr(clr, bg))
			{
				rect2.left = i;
				break;
			}
		}
		if(j!=(int)bmp2.GetHeight())
			break;
	}
	for(int i=(int)bmp2.GetWidth()-1; i>=0; i--)
	{
		int j;
		for(j=0; j<(int)bmp2.GetHeight(); j++)
		{
			Color clr;
			bmp2.GetPixel(i,j,&clr);
			if(!CompareClr(clr, bg))
			{
				rect2.right = i;
				break;
			}
		}
		if(j!=(int)bmp2.GetHeight())
			break;
	}
	for(int i=0; i<(int)bmp2.GetHeight(); i++)
	{
		int j;
		for(j=0; j<(int)bmp2.GetWidth(); j++)
		{
			Color clr;
			bmp2.GetPixel(j,i,&clr);
			if(!CompareClr(clr, bg))
			{
				rect2.top = i;
				break;
			}
		}
		if(j!=(int)bmp2.GetWidth())
			break;
	}
	for(int i=(int)bmp2.GetHeight()-1; i>=0; i--)
	{
		int j;
		for(j=0; j<(int)bmp2.GetWidth(); j++)
		{
			Color clr;
			bmp2.GetPixel(j,i,&clr);
			if(!CompareClr(clr, bg))
			{
				rect2.bottom = i;
				break;
			}
		}
		if(j!=(int)bmp2.GetWidth())
			break;
	}
	////////////////////////////////////////////////////////////////////////////

	Bitmap n_bmp2(rect2.right-rect2.left+1, rect2.bottom-rect2.top+1, bmp2.GetPixelFormat());

	for(int i = 0; i < n_bmp2.GetHeight(); i++)
	{
		for(int j = 0; j < n_bmp2.GetWidth(); j++)
		{
			Color clr;
			bmp2.GetPixel(j+rect2.left, i+rect2.top, &clr);
			n_bmp2.SetPixel(j,i,clr);
		}
	}

	wsprintf(new_filename,L"//vmware-host/Shared Folders/Downloads/classified/sorted/preproc/%d_%d.png",second,num*2+1);
	n_bmp2.Save(new_filename, &pngClsid, NULL);

	//RectF destination2(400, 0, (Gdiplus::REAL)n_bmp2.GetWidth(), (Gdiplus::REAL)n_bmp2.GetHeight());
	//G.DrawImage(&n_bmp2, destination2, 0, 0, (Gdiplus::REAL)n_bmp2.GetWidth(), (Gdiplus::REAL)n_bmp2.GetHeight(), UnitPixel, &ImgAttr);
}
void Img2Txt(WCHAR* filename, WCHAR* txt_filename, int h_max, int w_max)
{
	Bitmap bmp(filename, true);

	Color bg(192,192,192);
	Color color;

	int width = bmp.GetWidth();
	int height = bmp.GetHeight();

	FILE* fp = _wfopen(txt_filename, L"w");

	for(int i=0; i < h_max; i++)
	{
		for(int j=0; j < w_max; j++)
		{
			if( ((h_max - height)/2) <= i && i < ((h_max - height)/2)+height 
				&& ((w_max - width)/2) <= j && j < ((w_max - width)/2)+width )
			{
				Color clr;
				bmp.GetPixel(j-((w_max - width)/2), i-((h_max - height)/2), &clr);

				if(!CompareClr(clr, bg))
				{
					fwprintf(fp, L"%Lf ", (clr.GetR()*256*256+clr.GetG()*256+clr.GetB())/(256.0*256.0*256.0));
					continue;
				}
			}
			fwprintf(fp, L"%Lf ",0.0);
		}
		fwprintf(fp, L"\n");
	}
	fclose(fp);
}

void ReadTrainingData(WCHAR* filename, int ith, int h_max, int w_max)
{
	FILE* fp = _wfopen(filename,L"r");

	for(int i=0; i < h_max; i++)
	{
		for(int j=0; j < w_max; j++)
		{
			long double tmp;
			fwscanf(fp, L"%Lf", &tmp);
			X[ith].set(i,j,tmp);
		}
	}

	fclose(fp);
}

void ReadTestData(WCHAR* filename, int ith, int h_max, int w_max)
{
	FILE* fp = _wfopen(filename,L"r");

	for(int i=0; i < h_max; i++)
	{
		for(int j=0; j < w_max; j++)
		{
			long double tmp;
			fwscanf(fp, L"%Lf", &tmp);
			T_X[ith].set(i,j,tmp);
		}
	}

	fclose(fp);
}

long double kernel_func(Data x1, Data x2)
{
	//linear kernel
	if(Kernel_Mode == 0)
	{
		long double ret = 0;

		for(int i = 0; i < N; i++)
		{
			ret += x1.d1(i)*x2.d1(i);
		}
		return ret;
	}

	//gaussian kernel
	else //if(Kernel_Mode == 1)
	{
		long double ret = 0;
		long double sigma = 1;

		for(int i = 0; i < N; i++)
		{
			ret += (x1.d1(i)-x2.d1(i))*(x1.d1(i)-x2.d1(i));
		}
		return pow(E,-ret/(2*sigma*sigma));
	}
}

long double predict(Data x)
{
	if(Kernel_Mode == 0)
	{
		long double ret = 0;
		for(int i=0; i<N; i++)
		{
			ret += W[i]*x.d1(i);
		}
		ret += Bias;

		return ret;
	}
	else //if(Kernel_Mode == 1)
	{
		long double ret = 0;
		for(int i=0; i<M; i++)
		{
			ret += Alpha[i]*Y[i]*kernel_func(X[i],x);
		}
		ret += Bias;

		return ret;
	}
}

long double sigmoid(long double x)
{
	return 1/(1+pow(E,-10*x));
}