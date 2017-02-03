#include "bin_svm.h"
void binSVM::init(int loop, int data_num, int test_num, int kernel_mode)
{
	srand((unsigned)time(NULL));

	E = 2.718281828;
	C = 10000000.0;

	Kernel_Mode = kernel_mode;
	LOOP = loop;
	DATA_NUM = data_num;
	TEST_NUM = test_num;
	M = CLASS_NUM*DATA_NUM;

	W = vector<long double>(N);
	X = vector<Data>(M);
	Y = vector<int>(M);
	T_X = vector<Data>(CLASS_NUM*TEST_NUM);
	T_Y = vector<int>(CLASS_NUM*TEST_NUM);
	for(auto& it : X)
	{
		it = Data(N);
	}
	for(auto& it : T_X)
	{
		it = Data(N);
	}

	kernel_matrix = vector<vector<long double> >(M);
	for(auto& vec : kernel_matrix)
	{
		vec = vector<long double>(M);
	}
	Alpha = vector<long double>(M);
	predict_training = vector<long double>(M);

	Bias = 0;
	L = 0;
	oldL = 0;
}

binSVM::binSVM(int loop, int data_num, int test_num, int kernel_mode)
{
	init(loop, data_num, test_num, kernel_mode);
}

binSVM::binSVM(int loop, int data_num, int test_num, int n, int kernel_mode)
{
	N = n;
	init(loop, data_num, test_num, kernel_mode);
}

binSVM::binSVM(int loop, int data_num, int test_num, int n, int kernel_mode, const wstring data_filepath)
{
	N = n;
	FILEPATH = wstring(data_filepath);
	init(loop, data_num, test_num, kernel_mode);
}

void binSVM::read_data(int first, int second)
{
	WCHAR txt_filename[1024];
	int bin_class[2] = {first, second};

	for(int i = 0; i < CLASS_NUM; i++)
	{
		for(int j = 0; j < DATA_NUM; j++)
		{
			wsprintf(txt_filename,FILEPATH.c_str(),bin_class[i],j,L".txt");
			/*wsprintf(filename,FILEPATH.c_str(),bin_class[i],j,L".png");
			if(_waccess(filename, 0) != 0)
			{
				Img2Txt(filename, txt_filename, h_max, w_max);
			}*/
			read_training_data(txt_filename, i*(DATA_NUM)+j);
				
			// class is 1 if i==0, class is -1 if i==1
			Y[i*(DATA_NUM)+j] = -2*i+1;
		}
	}

	printf("input trainig done\n");
	fflush(stdout);



	for(int i = 0; i < CLASS_NUM; i++)
	{
		for(int j = 0; j < TEST_NUM; j++)
		{
			wsprintf(txt_filename,FILEPATH.c_str(),bin_class[i], j+DATA_NUM, L".txt");
			/*wsprintf(filename,FILEPATH.c_str(),bin_class[i], j+DATA_NUM, L".png");
			if(_waccess(filename, 0) != 0)
			{
				Img2Txt(filename, txt_filename, h_max, w_max);
			}*/
			read_test_data(txt_filename, i*(TEST_NUM)+j);
				
			// class is 1 if i==0, class is -1 if i==1
			T_Y[i*(TEST_NUM)+j] = -2*i+1;
		}
	}

	printf("input test done\n");
	fflush(stdout);
}

void binSVM::read_data(int first, int second, const wstring data_filepath)
{
	FILEPATH = wstring(data_filepath);
	read_data(first, second);
}

void binSVM::read_training_data(WCHAR* filename, int ith)
{
	FILE* fp = _wfopen(filename, L"r");

	
	for(int i = 0; i < N; i++)
	{
		long double temp;
		fwscanf(fp, L"%Lf", &temp);
		X[ith].set(i, temp);
	}
	
	fclose(fp);
}

void binSVM::read_test_data(WCHAR* filename, int ith)
{
	FILE* fp = _wfopen(filename,L"r");

	for(int i = 0; i < N; i++)
	{
		long double temp;
		fwscanf(fp, L"%Lf", &temp);
		T_X[ith].set(i, temp);
	}


	fclose(fp);
}

void binSVM::init_kernel_matrix()
{
	time_t crtime;
	crtime = time(0);
	printf(ctime(&crtime));
	fflush(stdout);
	for(int i = 0; i<M*M; i++)
	{
		kernel_matrix[i/M][i%M] = kernel_func(X[i/M],X[i%M]);
	}
	crtime = time(0);
	printf(ctime(&crtime));
	printf("kernel matrix done\n");
	fflush(stdout);
}

long double binSVM::kernel_func(Data& x1, Data& x2)
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

long double binSVM::predict(Data& x)
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

void binSVM::training(long double relative_error)
{
	unsigned long long loop = 0;

	while(1)
	{
		static int one,two;
		long double E1;
		long double E2;
			

		/*one = rand()%M;
		two = rand()%(M-1);
		if(one == two)
			two = M-1;*/
				

		//pick two alpha heuristically
		if(loop==0)
		{
			one = -1;
		}

		while(1)
		{
			one++;
			if(one==M)
			{
				one = 0;
				break;
			}
				
			if(abs(Alpha[one]) < 1e-8)
			{
				if(Y[one]*predict_training[one] < 0) //1
					break;
			}
			else if(abs(Alpha[one]-C) < 1e-8)
			{
				if(Y[one]*predict_training[one] > 0) //1
					break;
			}
			else
			{
				if(abs(Y[one]*predict_training[one] - 1) > 1e-8)
					break;
			}
		}
			
		two = rand()%(M-1);
		if(one == two)
			two = M-1;
		



		//update alpha by SMO
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

		E1 = predict_training[one] - Y[one];
		E2 = predict_training[two] - Y[two];
		long double K = kernel_matrix[one][one] + kernel_matrix[two][two] - 2 * kernel_matrix[one][two];
		if(abs(K)<1e-8)
		{
			continue;
		}
		long double Alpha_two_new_unc = Alpha[two] + (Y[two] * (E1-E2) / K);
		long double Alpha_two_new;

		if (Alpha_two_new_unc > V)
			Alpha_two_new = V;
		else if (Alpha_two_new_unc < U)
			Alpha_two_new = U;
		else
			Alpha_two_new = Alpha_two_new_unc;

		long double Alpha_one_new = Alpha[one] + Y[one]*Y[two]*(Alpha[two] - Alpha_two_new);

		//get W (cached)
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

		//get preditcted value of training data (cached)
		for(int i = 0; i<M; i++)
		{
			predict_training[i] -= Alpha[one]*Y[one]*kernel_matrix[one][i];
			predict_training[i] -= Alpha[two]*Y[two]*kernel_matrix[two][i];
			predict_training[i] += Alpha_one_new*Y[one]*kernel_matrix[one][i];
			predict_training[i] += Alpha_two_new*Y[two]*kernel_matrix[two][i];
		}

		Alpha[one] = Alpha_one_new;
		Alpha[two] = Alpha_two_new;

		loop++;





		//test if L converges so that the loop can be escaped. checks on every LOOPth loops.
		if(loop%LOOP==1)
		{
			L = 0.0;
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

			//escape condition
			if(loop > LOOP*10)
			{
				printf("%d %.20Lf\n",(int)loop,L);
				fflush(stdout);
				if(L - oldL <= L*relative_error)
					break;
			}
			oldL = L;
		}
	}

	time_t crtime = time(0);
	printf(ctime(&crtime));
	printf("alpha done\n");
	fflush(stdout);

	calc_bias();
}

void binSVM::calc_bias()
{
	//get Bias
	long double ret_sum = 0;
	int ret_cnt = 0;
	for(int i = 0; i < M; i++)
	{
		if (1e-5 < Alpha[i] && Alpha[i] < C - 1e-5)
		{
			long double ret = (1 - Y[i]*(predict(X[i])-Bias))/Y[i];
			ret_sum += ret;
			ret_cnt++;
		}
	}
	printf("Bias: %Lf\n",Bias = ret_sum/(long double)ret_cnt);

	for(int i = 0; i<M; i++)
		predict_training[i] += Bias;
}

void binSVM::testing()
{
	int wrong_count = 0;
	long double error_sum = 0;
	for(int i = 0; i<M; i++)
	{
		long double predicted = sigmoid(predict(X[i]))*2-1;
		if((Y[i]==1 && predicted < 0.5) || (Y[i]==-1 && predicted > -0.5))
			wrong_count++;
		error_sum += abs(predicted - Y[i]);
	}

	int test_wrong_count = 0;
	long double test_error_sum = 0;
	for(int i = 0; i<CLASS_NUM*TEST_NUM; i++)
	{
		long double predicted = sigmoid(predict(T_X[i]))*2-1;
		if((T_Y[i]==1 && predicted < 0.5) || (T_Y[i]==-1 && predicted > -0.5))
			test_wrong_count++;
		test_error_sum += abs(predicted - T_Y[i]);
	}

	printf("L: %Lf   wrong:%d/%d(=%Lf)  error_sum(cost):%Lf    error_avg:%Lf    /test:/  wrong:%d/%d(=%Lf)  error_sum(cost):%Lf    error_avg:%Lf\n",
		L,wrong_count,M,(long double)wrong_count/M, error_sum, error_sum/(long double)M,
		test_wrong_count,CLASS_NUM*TEST_NUM,(long double)test_wrong_count/(CLASS_NUM*TEST_NUM), test_error_sum, test_error_sum/(long double)(CLASS_NUM*TEST_NUM));
	fflush(stdout);

	FILE* fp = fopen("alpha.txt","w");
	for(int i=0; i<M; i++)
	{
		fprintf(fp, "%Lf\n",Alpha[i]);
	}
	fclose(fp);
}


long double binSVM::sigmoid(long double x)
{
	return 1/(1+pow(E,-10*x));
}