/*
#ifndef __BIN_IMG_SVM__
#define __BIN_IMG_SVM__

#include <Windows.h>
#include <gdiplus.h>
#include "bin_svm.h"

#pragma warning(disable:4996)

using namespace Gdiplus;
#pragma comment(lib, "gdiplus")

class binIMG_SVM : public binSVM
{
private:
	int w_max, h_max;
	bool CompareClr(Color A, Color B);
	int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);
protected:
public:
	binIMG_SVM(int loop, int data_num, int test_num, int h, int w, int kernel_mode);
	binIMG_SVM(int loop, int data_num, int test_num, int h, int w, int kernel_mode, const wstring data_filepath);
	binIMG_SVM(int class_num, int loop, int data_num, int test_num, int h, int w, int kernel_mode, const wstring data_filepath);
	void read_data(int first);
	void read_data(int first, int second);
	void add_data(int first, int num);
	
	void PreProc(HDC hdc, WCHAR* filename, int first, int second, int num);
	void img2txt(WCHAR* filename, WCHAR* txt_filename);
};

#endif
*/