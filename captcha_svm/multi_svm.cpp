#include "multi_svm.h"

multiSVM::multiSVM(int class_num, int loop, int data_num, int test_num, int n, int kernel_mode, const wstring data_filepath) : binSVM(class_num, loop, data_num, test_num, n, kernel_mode, data_filepath)
{
	for(auto& it : *X)
	{
		it = Data(N);
	}
	for(auto& it : *T_X)
	{
		it = Data(N);
	}
	for(auto& vec : *kernel_matrix)
	{
		vec = vector<long double>(M);
	}


	svm = vector<binSVM> (class_num);
	for(auto& it : svm)
	{
		it = binSVM(class_num, loop, data_num, test_num, n, kernel_mode, data_filepath);
	}
}

void multiSVM::read_data()
{
	WCHAR txt_filename[1024];

	for(int i = 0; i < CLASS_NUM; i++)
	{
		for(int j = 0; j < DATA_NUM; j++)
		{
			wsprintf(txt_filename,FILEPATH.c_str(), i, j, L".txt");
			read_training_data(txt_filename, i*(DATA_NUM)+j);
		}
	}

	printf("input trainig done\n");
	fflush(stdout);



	for(int i = 0; i < CLASS_NUM; i++)
	{
		for(int j = 0; j < TEST_NUM; j++)
		{
			wsprintf(txt_filename,FILEPATH.c_str(), i, j+DATA_NUM, L".txt");
			read_test_data(txt_filename, i*(TEST_NUM)+j);
		}
	}

	printf("input test done\n");
	fflush(stdout);

	for(int i = 0; i < CLASS_NUM; i++)
	{
		svm[i].copy_training(X);
		svm[i].copy_test(T_X);
		svm[i].setY(i);
	}
}


void multiSVM::init_kernel_matrix()
{
	binSVM::init_kernel_matrix();

	for(auto& it : svm)
	{
		it.copy_kernel_matrix(kernel_matrix);
	}
}

void multiSVM::training(long double relative_error)
{
	for(auto& it : svm)
	{
		it.training(relative_error);
	}
}

/*BOOL multiSVM::training(long double relative_error, HDC MemDC)
{
	static int i = 0;
	
	if(i==CLASS_NUM)
	{
		return false;
	}

	if(svm[i].training(relative_error, MemDC))
	{
		time_t crtime = time(0);
		printf(ctime(&crtime));
		printf("alpha done\n");
		fflush(stdout);

		svm[i].calc_bias();
		i++;
	}
	return true;
}*/

void multiSVM::testing()
{
	for(int i=0; i<CLASS_NUM; i++)
	{
		svm[i].testing(i);
	}
}