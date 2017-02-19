#ifndef __MULTI_SVM__
#define __MULTI_SVM__

#include <vector>
#include "bin_svm.h"

using namespace std;

class multiSVM : public binSVM
{
protected:
	vector<binSVM> svm;
public:
	multiSVM() {};
	multiSVM(int class_num, int loop, int data_num, int test_num, int n, int kernel_mode, const wstring data_filepath);
	void read_data();
	void init_kernel_matrix();
	void training(long double relative_error);
	//BOOL training(long double relative_error, HDC MemDC);
	void testing();
};

#endif