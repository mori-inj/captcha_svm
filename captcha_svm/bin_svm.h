#ifndef __BIN_SVM__
#define __BIN_SVM__

#include <Windows.h>
#include <io.h>
#include <time.h>
#include <vector>
#include "data.h"

using namespace std;

class binSVM
{
private:
	//binary svm
	const int CLASS_NUM = 2;
	int Kernel_Mode;

	const long double E = 2.718281828;
	const long double C = 10000000.0;				//used for soft margin svm

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

protected:
	void read_training_data(WCHAR* filename, int ith);
	void read_test_data(WCHAR* filename, int ith);
	void calc_bias();

public:
	binSVM() {};
	binSVM(int loop, int data_num, int test_num, int n, int kernel_mode);
	binSVM(int loop, int data_num, int test_num, int n, int kernel_mode, wstring data_filepath);
	void read_data(int first, int second);
	void read_data(int first, int second, wstring data_filepath);
	void init_kernel_matrix();
	long double kernel_func(Data& x1, Data& x2);
	long double predict(Data& x);
	void training(long double relative_error);
	void testing();

	long double sigmoid(long double x);
};

#endif