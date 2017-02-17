/*
#include "bin_img_svm.h"

binIMG_SVM::binIMG_SVM(int loop, int data_num, int test_num, int h, int w, int kernel_mode)
{
	h_max = h;
	w_max = w;
	N = 3*h*w;
	init(2, loop, data_num, test_num, kernel_mode);
}

binIMG_SVM::binIMG_SVM(int loop, int data_num, int test_num, int h, int w, int kernel_mode, const wstring data_filepath)
{
	h_max = h;
	w_max = w;
	N = 3*h*w;
	FILEPATH = wstring(data_filepath);
	init(2, loop, data_num, test_num, kernel_mode);
}

binIMG_SVM::binIMG_SVM(int class_num, int loop, int data_num, int test_num, int h, int w, int kernel_mode, const wstring data_filepath)
{
	h_max = h;
	w_max = w;
	N = 3*h*w;
	FILEPATH = wstring(data_filepath);
	init(class_num, loop, data_num, test_num, kernel_mode);
}

void binIMG_SVM::read_data(int first)
{
	this->class1 = first;

	WCHAR filename[1024], txt_filename[1024];

	for(int i = 0; i < CLASS_NUM; i++)
	{
		for(int j = 0; j < DATA_NUM; j++)
		{
			wsprintf(txt_filename,FILEPATH.c_str(), i, j, L".txt");
			wsprintf(filename,FILEPATH.c_str(), i, j, L".png");
			if(_waccess(txt_filename, 0) != 0)
			{
				img2txt(filename, txt_filename);
			}
			read_training_data(txt_filename, i*(DATA_NUM)+j);
				
			if(i==first)
				Y[i*(DATA_NUM)+j] = 1;
			else
				Y[i*(DATA_NUM)+j] = -1;
		}
	}

	printf("input trainig done\n");
	fflush(stdout);



	for(int i = 0; i < CLASS_NUM; i++)
	{
		for(int j = 0; j < TEST_NUM; j++)
		{
			wsprintf(txt_filename,FILEPATH.c_str(), i, j+DATA_NUM, L".txt");
			wsprintf(filename,FILEPATH.c_str(), i, j+DATA_NUM, L".png");
			if(_waccess(txt_filename, 0) != 0)
			{
				img2txt(filename, txt_filename);
			}
			read_test_data(txt_filename, i*(TEST_NUM)+j);
				
			if(i==first)
				T_Y[i*(TEST_NUM)+j] = 1;
			else
				T_Y[i*(TEST_NUM)+j] = -1;
		}
	}

	printf("input test done\n");
	fflush(stdout);
}

void binIMG_SVM::read_data(int first, int second)
{
	this->class1 = first;
	this->class2 = second;

	WCHAR filename[1024], txt_filename[1024];
	int bin_class[2] = {first, second};

	for(int i = 0; i < CLASS_NUM; i++)
	{
		for(int j = 0; j < DATA_NUM; j++)
		{
			wsprintf(txt_filename,FILEPATH.c_str(),bin_class[i],j,L".txt");
			wsprintf(filename,FILEPATH.c_str(),bin_class[i],j,L".png");
			if(_waccess(txt_filename, 0) != 0)
			{
				img2txt(filename, txt_filename);
			}
			read_training_data(txt_filename, i*(DATA_NUM)+j);
				
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
		for(int j = 0; j < TEST_NUM; j++)
		{
			wsprintf(txt_filename,FILEPATH.c_str(),bin_class[i], j+DATA_NUM, L".txt");
			wsprintf(filename,FILEPATH.c_str(),bin_class[i], j+DATA_NUM, L".png");
			if(_waccess(txt_filename, 0) != 0)
			{
				img2txt(filename, txt_filename);
			}
			read_test_data(txt_filename, i*(TEST_NUM)+j);
				
			if(i==0)
				T_Y[i*(TEST_NUM)+j] = 1;
			else
				T_Y[i*(TEST_NUM)+j] = -1;
		}
	}

	printf("input test done\n");
	fflush(stdout);
}

void binIMG_SVM::add_data(int first, int num)
{
	WCHAR txt_filename[1024], filename[1024];
	
	
	for(int i = 0; i<num; i++)
	{
		wsprintf(txt_filename,FILEPATH.c_str(), first, DATA_NUM+i, L".txt");
		wsprintf(filename,FILEPATH.c_str(), first, DATA_NUM+i, L".png");
		if(_waccess(txt_filename, 0) != 0)
		{
			img2txt(filename, txt_filename);
		}
		X.push_back(Data(N));
		read_training_data(txt_filename, M+i);
		if(first == this->class1)
			Y.push_back(1);
		else
			Y.push_back(-1);
	}
	
	M += num;
	for(int i=0; i<num; i++)
	{
		W.push_back(0);
		Alpha.push_back(0);
		predict_training.push_back(0);
	}

	for(auto& vec : kernel_matrix)
	{
		for(int i=0; i<num; i++)
			vec.push_back(0);
	}

	for(int i=0; i<num; i++)
		kernel_matrix.push_back(vector<long double>(M));

	for(int i=0; i<M-num; i++)
	{
		for(int j=M-num; j<M; j++)
			kernel_matrix[i][j] = kernel_func(X[i],X[j]);
	}
	for(int i=M-num; i<M; i++)
	{
		for(int j=0; j<M; j++)
			kernel_matrix[i][j] = kernel_func(X[i],X[j]);
	}

}

bool binIMG_SVM::CompareClr(Color A, Color B)
{
	if(A.GetR() == B.GetR() && A.GetG() == B.GetG() && A.GetB() == B.GetB())
		return true;
	return false;
}

void binIMG_SVM::PreProc(HDC hdc, WCHAR* filename, int first, int second, int num)
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

//code from MSDN(https://msdn.microsoft.com/en-us/library/windows/desktop/ms533843(v=vs.85).aspx)
int binIMG_SVM::GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
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

void binIMG_SVM::img2txt(WCHAR* filename, WCHAR* txt_filename)
{
	Bitmap bmp(filename, true);

	//Color bg(192,192,192);
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

				//if(!CompareClr(clr, bg))
				//{
					//fwprintf(fp,L"%Lf ",(long double)(clr.GetR()*256*256+clr.GetG()*256+clr.GetB())/(256*256*256));
					fwprintf(fp, L"%Lf %Lf %Lf ", (long double)((int)clr.GetR())/256, (long double)((int)clr.GetG())/256, (long double)((int)clr.GetB())/256);
					continue;
				//}
			}
			//fwprintf(fp, L"%Lf ",0.0);
			fwprintf(fp, L"%Lf %Lf %Lf ",0.0,0.0,0.0);
		}
		fwprintf(fp, L"\n");
	}
	fclose(fp);
}

*/