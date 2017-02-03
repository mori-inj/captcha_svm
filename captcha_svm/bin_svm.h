#ifndef __BIN_SVM__
#define __BIN_SVM__

#include <Windows.h>
#include <io.h>
#include <time.h>
#include <vector>
#include "data.h"

#pragma warning(disable:4996)
using namespace std;

class binSVM
{
protected:
	//binary svm
	static const int CLASS_NUM = 2;
	int Kernel_Mode;

	long double E;
	long double C;				//used for soft margin svm

	int LOOP;		//# of iterations per each loop escape test

	int DATA_NUM;
	int TEST_NUM;
	int M;			// # of training data = DATA_NUM*TEST_NUM

	int N;			// dimension of data

	vector<long double> W;			//weight
	vector<Data> X;					//training data
	vector<int> Y;					//class of training data
	vector<Data> T_X;				//test data
	vector<int> T_Y;				//class of training data

	vector<vector<long double> > kernel_matrix;		//cached K(X[i],X[j])
	vector<long double> Alpha;		//alpha (weight used in SMO)
	vector<long double> predict_training;			//cached predicted class of training data
	long double Bias;
	long double L;					// L(w, b, alpha)
	long double oldL;

	wstring FILEPATH;

	void read_training_data(WCHAR* filename, int ith);
	void read_test_data(WCHAR* filename, int ith);
	void calc_bias();

public:
	binSVM() {};
	void init(int loop, int data_num, int test_num, int kernel_mode);
	binSVM(int loop, int data_num, int test_num, int kernel_mode);
	binSVM(int loop, int data_num, int test_num, int n, int kernel_mode);
	binSVM(int loop, int data_num, int test_num, int n, int kernel_mode, const wstring data_filepath);
	void read_data(int first, int second);
	void read_data(int first, int second, const wstring data_filepath);
	void init_kernel_matrix();
	long double kernel_func(Data& x1, Data& x2);
	long double predict(Data& x);
	void training(long double relative_error);
	void testing();

	long double sigmoid(long double x);
};

#endif