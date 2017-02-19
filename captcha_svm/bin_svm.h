#ifndef __BIN_SVM__
#define __BIN_SVM__

#include <Windows.h>
#include <stdio.h>
#include <assert.h>
#include <io.h>
#include <time.h>
#include <vector>
#include "data.h"

#pragma warning(disable:4996)
using namespace std;

//binary svm
class binSVM
{
protected:
	int CLASS_NUM;				//automatically set to 2
	int Kernel_Mode;

	long double E;				//2.718281828
	long double C;				//used for soft margin svm

	int LOOP;		//# of iterations per each loop escape test

	int DATA_NUM;
	int TEST_NUM;
	int M;			// # of training data = DATA_NUM*TEST_NUM

	int N;			// dimension of data

	int class1, class2;

	vector<long double> W;			//weight. size: N
	vector<Data>* X;				//training data. size: M
	vector<long double>* Y;			//class of training data. size: M
	vector<Data>* T_X;				//test data.
	vector<long double>* T_Y;		//class of training data

	vector<vector<long double> >* kernel_matrix;		//cached K(X[i],X[j]). size: M*M
	vector<long double> Alpha;						//alpha (weight used in SMO). size: M
	vector<long double> predict_training;			//cached predicted class of training data. size: M
	long double Bias;
	long double L;					// L(w, b, alpha)
	long double oldL;

	wstring FILEPATH;

	void read_training_data(WCHAR* filename, int ith);
	void read_test_data(WCHAR* filename, int ith);

public:
	binSVM() {};
	binSVM(int class_num, int loop, int data_num, int test_num, int n, int kernel_mode, const wstring data_filepath);
	void copy_training(vector<Data>* X_source);
	void copy_test(vector<Data>* T_X_source);
	void copy_kernel_matrix(vector<vector<long double> >* kernel_matrix);
	void read_data(int first);
	void read_data(int first, int second);
	void setY(int first);
	void add_data(int first, int num);
	void add_data(int first, Data Sample);
	void init_kernel_matrix();
	long double kernel_func(Data& x1, Data& x2);
	long double predict(Data& x);
	void training(long double relative_error);
	//BOOL training(long double relative_error, HDC MemDC);
	void calc_bias();
	void testing(int idx);

	long double sigmoid(long double x);
};

#endif