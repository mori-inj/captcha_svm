#include "bin_svm.h"

binSVM::binSVM(int class_num, int loop, int data_num, int test_num, int n, int kernel_mode, const wstring data_filepath)
{
	N = n;
	FILEPATH = wstring(data_filepath);
	CLASS_NUM = class_num;
	srand((unsigned)time(NULL));

	E = 2.718281828;
	C = 10000000.0;

	Kernel_Mode = kernel_mode;
	LOOP = loop;
	DATA_NUM = data_num;
	TEST_NUM = test_num;
	M = CLASS_NUM*DATA_NUM;

	W = vector<long double>(N);
	X = new vector<Data>(M);
	Y = new vector<long double>(M);
	T_X = new vector<Data>(CLASS_NUM*TEST_NUM);
	T_Y = new vector<long double>(CLASS_NUM*TEST_NUM);
	/*for(auto& it : *X)
	{
		it = Data(N);
	}
	for(auto& it : *T_X)
	{
		it = Data(N);
	}*/

	kernel_matrix = new vector<vector<long double> >(M);
	/*for(auto& vec : *kernel_matrix)
	{
		vec = vector<long double>(M);
	}*/
	Alpha = vector<long double>(M);
	predict_training = vector<long double>(M);

	Bias = 0;
	L = 0;
	oldL = 0;
}

void binSVM::copy_training(vector<Data>* X_source)
{
	X = X_source;
}

void binSVM::copy_test(vector<Data>* T_X_source)
{
	T_X = T_X_source;
}

void binSVM::copy_kernel_matrix(vector<vector<long double> >* kernel_matrix)
{
	this->kernel_matrix = kernel_matrix;
}

void binSVM::read_data(int first)
{
	this->class1 = first;

	WCHAR txt_filename[1024];

	for(int i = 0; i < CLASS_NUM; i++)
	{
		for(int j = 0; j < DATA_NUM; j++)
		{
			wsprintf(txt_filename,FILEPATH.c_str(), i, j, L".txt");
			read_training_data(txt_filename, i*(DATA_NUM)+j);
				
			if(i==first)
				(*Y)[i*(DATA_NUM)+j] = 1;
			else
				(*Y)[i*(DATA_NUM)+j] = -1;

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
				
			if(i==first)
				(*T_Y)[i*(TEST_NUM)+j] = 1;
			else
				(*T_Y)[i*(TEST_NUM)+j] = -1;
		}
	}

	printf("input test done\n");
	fflush(stdout);
}

void binSVM::read_data(int first, int second)
{
	this->class1 = first;
	this->class2 = second;

	WCHAR txt_filename[1024];
	int bin_class[2] = {first, second};

	for(int i = 0; i < CLASS_NUM; i++)
	{
		for(int j = 0; j < DATA_NUM; j++)
		{
			wsprintf(txt_filename,FILEPATH.c_str(),bin_class[i],j,L".txt");
			read_training_data(txt_filename, i*(DATA_NUM)+j);
				
			if(i==first)
				(*Y)[i*(DATA_NUM)+j] = 1;
			else
				(*Y)[i*(DATA_NUM)+j] = -1;

		}
	}

	printf("input trainig done\n");
	fflush(stdout);



	for(int i = 0; i < CLASS_NUM; i++)
	{
		for(int j = 0; j < TEST_NUM; j++)
		{
			wsprintf(txt_filename,FILEPATH.c_str(),bin_class[i], j+DATA_NUM, L".txt");
			read_test_data(txt_filename, i*(TEST_NUM)+j);
				
			if(i==first)
				(*T_Y)[i*(TEST_NUM)+j] = 1;
			else
				(*T_Y)[i*(TEST_NUM)+j] = -1;
		}
	}

	printf("input test done\n");
	fflush(stdout);
}

void binSVM::setY(int first)
{
	this->class1 = first;

	char out_filename[1024];
	sprintf(out_filename, "training_error(%d).txt",first);
	FILE* fp = fopen(out_filename,"r");

	for(int i = 0; i < CLASS_NUM; i++)
	{
		for(int j = 0; j < DATA_NUM; j++)
		{
			long double error;
			long double ret;
			long double error_max = 0.0001, thres = 0.1;
			fscanf(fp,"%Lf",&error);

			if(error > error_max)
				ret = thres;
			else
				ret = (-(1.0-thres)/error_max)*error + 1.0;

			assert(ret>=thres);

			if(i==first)
				(*Y)[i*(DATA_NUM)+j] = ret;
			else
				(*Y)[i*(DATA_NUM)+j] = -ret;

		}
	}

	fclose(fp);


	for(int i = 0; i < CLASS_NUM; i++)
	{
		for(int j = 0; j < TEST_NUM; j++)
		{
				
			if(i==first)
				(*T_Y)[i*(TEST_NUM)+j] = 1;
			else
				(*T_Y)[i*(TEST_NUM)+j] = -1;
		}
	}
}

void binSVM::read_training_data(WCHAR* filename, int ith)
{
	FILE* fp = _wfopen(filename, L"r");

	
	for(int i = 0; i < N; i++)
	{
		long double temp;
		fwscanf(fp, L"%Lf", &temp);
		(*X)[ith].set(i, temp);
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
		(*T_X)[ith].set(i, temp);
	}


	fclose(fp);
}

void binSVM::add_data(int first, int num)
{
	WCHAR txt_filename[1024];
	
	
	for(int i = 0; i<num; i++)
	{
		wsprintf(txt_filename,FILEPATH.c_str(), first, DATA_NUM+TEST_NUM+i, L".txt");
		X->push_back(Data(N));
		read_training_data(txt_filename, M+i);
		if(first == this->class1)
			Y->push_back(1);
		else
			Y->push_back(-1);
	}
	
	M += num;
	for(int i=0; i<num; i++)
	{
		W.push_back(0);
		Alpha.push_back(0);
		predict_training.push_back(0);
	}

	for(auto& vec : *kernel_matrix)
	{
		for(int i=0; i<num; i++)
			vec.push_back(0);
	}

	for(int i=0; i<num; i++)
		kernel_matrix->push_back(vector<long double>(M));

	for(int i=0; i<M-num; i++)
	{
		for(int j=M-num; j<M; j++)
			(*kernel_matrix)[i][j] = kernel_func((*X)[i], (*X)[j]);
	}
	for(int i=M-num; i<M; i++)
	{
		for(int j=0; j<M; j++)
			(*kernel_matrix)[i][j] = kernel_func((*X)[i], (*X)[j]);
	}

}

void binSVM::add_data(int first, Data Sample)
{
	X->push_back(Sample);
	if(first == this->class1)
		Y->push_back(1);
	else
		Y->push_back(-1);

	M ++;
	
	W.push_back(0);
	Alpha.push_back(0);
	predict_training.push_back(0);
	

	for(auto& vec : *kernel_matrix)
	{
		vec.push_back(0);
	}

	
	kernel_matrix->push_back(vector<long double>(M));

	for(int i=0; i<M-1; i++)
	{
		for(int j=M-1; j<M; j++)
			(*kernel_matrix)[i][j] = kernel_func((*X)[i], (*X)[j]);
	}
	for(int i=M-1; i<M; i++)
	{
		for(int j=0; j<M; j++)
			(*kernel_matrix)[i][j] = kernel_func((*X)[i], (*X)[j]);
	}
}

void binSVM::init_kernel_matrix()
{
	time_t crtime;
	crtime = time(0);
	printf(ctime(&crtime));
	fflush(stdout);
	for(int i = 0; i<M*M; i++)
	{
		(*kernel_matrix)[i/M][i%M] = kernel_func((*X)[i/M], (*X)[i%M]);
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
			ret += Alpha[i] * (*Y)[i] * kernel_func((*X)[i], x);
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
				if((*Y)[one] * predict_training[one] < 0) //1
					break;
			}
			else if(abs(Alpha[one]-C) < 1e-8)
			{
				if((*Y)[one] * predict_training[one] > 0) //1
					break;
			}
			else
			{
				if(abs((*Y)[one] * predict_training[one] - 1) > 1e-8)
					break;
			}
		}
			
		two = rand()%(M-1);
		if(one == two)
			two = M-1;



		//update alpha by SMO
		long double U,V;
		if ((*Y)[one] != (*Y)[two])
		{
			U = max(0, Alpha[two] - Alpha[one]);
			V = min(C, C - Alpha[one] + Alpha[two]);
		}
		else
		{
			U = max(0, Alpha[one] + Alpha[two] - C);
			V = min(C, Alpha[one] + Alpha[two]);
		}

		E1 = predict_training[one] - 1.0/(*Y)[one];
		E2 = predict_training[two] - 1.0/(*Y)[two];
		long double K = (*kernel_matrix)[one][one] 
						+ (*kernel_matrix)[two][two] 
						- 2 * (*kernel_matrix)[one][two];
		if(abs(K)<1e-8)
		{
			continue;
		}
		long double Alpha_two_new_unc = Alpha[two] + ( (1.0/(*Y)[two]) * (E1-E2) / K);
		long double Alpha_two_new;

		if (Alpha_two_new_unc > V)
			Alpha_two_new = V;
		else if (Alpha_two_new_unc < U)
			Alpha_two_new = U;
		else
			Alpha_two_new = Alpha_two_new_unc;

		long double Alpha_one_new = Alpha[one] + (*Y)[one] * (*Y)[two] * (Alpha[two] - Alpha_two_new);

		//get W (cached)
		if (Kernel_Mode == 0)
		{
			for(int i=0; i<N; i++)
			{
				W[i] -= Alpha[one] * (*Y)[one] * (*X)[one].d1(i);
				W[i] -= Alpha[two] * (*Y)[two] * (*X)[two].d1(i);

				W[i] += Alpha_one_new * (*Y)[one] * (*X)[one].d1(i);
				W[i] += Alpha_two_new * (*Y)[two] * (*X)[two].d1(i);
			}
		}

		//get preditcted value of training data (cached)
		for(int i = 0; i<M; i++)
		{
			predict_training[i] -= Alpha[one] * (*Y)[one] * (*kernel_matrix)[one][i];
			predict_training[i] -= Alpha[two] * (*Y)[two] * (*kernel_matrix)[two][i];
			predict_training[i] += Alpha_one_new * (*Y)[one] * (*kernel_matrix)[one][i];
			predict_training[i] += Alpha_two_new * (*Y)[two] * (*kernel_matrix)[two][i];
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
						ret += Alpha[i] * Alpha[j] * (*Y)[i] * (*Y)[j] * (*kernel_matrix)[i][j];
					}
				}
			}
			L -= 0.5*ret;

			//escape condition
			if(loop > LOOP*10)
			{
				if(L - oldL <= L*relative_error)
					break;
			}
			oldL = L;
			printf("%10lu: %Lf\n",loop, L);
			fflush(stdout);
		}
	}

	time_t crtime = time(0);
	printf(ctime(&crtime));
	printf("alpha done\n");
	fflush(stdout);

	calc_bias();
}

/*BOOL binSVM::training(long double relative_error, HDC MemDC)
{
	static unsigned long long loop = 0;
	static HPEN hPen = CreatePen(PS_SOLID, 5, RGB(255, 255, 255)), oldPen,
		data1Pen = CreatePen(PS_SOLID, 1, RGB(0, 0, 255)),
		data2Pen = CreatePen(PS_SOLID, 1, RGB(0, 255, 0)),
		linePen = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));

	//while(1)
	//{
		static int one,two;
		long double E1;
		long double E2;

				

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
				if((*Y)[one] * predict_training[one] < 0) //1
					break;
			}
			else if(abs(Alpha[one]-C) < 1e-8)
			{
				if((*Y)[one] * predict_training[one] > 0) //1
					break;
			}
			else
			{
				if(abs((*Y)[one] * predict_training[one] - 1) > 1e-8)
					break;
			}
		}
			
		two = rand()%(M-1);
		if(one == two)
			two = M-1;



		//update alpha by SMO
		long double U,V;
		if ((*Y)[one] != (*Y)[two])
		{
			U = max(0, Alpha[two] - Alpha[one]);
			V = min(C, C - Alpha[one] + Alpha[two]);
		}
		else
		{
			U = max(0, Alpha[one] + Alpha[two] - C);
			V = min(C, Alpha[one] + Alpha[two]);
		}

		E1 = predict_training[one] - (*Y)[one];
		E2 = predict_training[two] - (*Y)[two];
		long double K = (*kernel_matrix)[one][one] 
						+ (*kernel_matrix)[two][two] 
						- 2 * (*kernel_matrix)[one][two];
		if(abs(K)<1e-8)
		{
			return false; //continue;
		}
		long double Alpha_two_new_unc = Alpha[two] + ((*Y)[two] * (E1-E2) / K);
		long double Alpha_two_new;

		if (Alpha_two_new_unc > V)
			Alpha_two_new = V;
		else if (Alpha_two_new_unc < U)
			Alpha_two_new = U;
		else
			Alpha_two_new = Alpha_two_new_unc;

		long double Alpha_one_new = Alpha[one] + (*Y)[one] * (*Y)[two] * (Alpha[two] - Alpha_two_new);

		//get W (cached)
		if (Kernel_Mode == 0)
		{
			for(int i=0; i<N; i++)
			{
				W[i] -= Alpha[one] * (*Y)[one] * (*X)[one].d1(i);
				W[i] -= Alpha[two] * (*Y)[two] * (*X)[two].d1(i);

				W[i] += Alpha_one_new * (*Y)[one] * (*X)[one].d1(i);
				W[i] += Alpha_two_new * (*Y)[two] * (*X)[two].d1(i);
			}
		}

		//get preditcted value of training data (cached)
		for(int i = 0; i<M; i++)
		{
			predict_training[i] -= Alpha[one] * (*Y)[one] * (*kernel_matrix)[one][i];
			predict_training[i] -= Alpha[two] * (*Y)[two] * (*kernel_matrix)[two][i];
			predict_training[i] += Alpha_one_new * (*Y)[one] * (*kernel_matrix)[one][i];
			predict_training[i] += Alpha_two_new * (*Y)[two] * (*kernel_matrix)[two][i];
		}

		Alpha[one] = Alpha_one_new;
		Alpha[two] = Alpha_two_new;

		loop++;

		


		if(loop%1==0)
		{
			int one = 100, two = 200;
			oldPen = (HPEN)SelectObject(MemDC, hPen);
			MoveToEx(MemDC, 800/2-200, 600/2+200, NULL);
			LineTo(MemDC, 800/2-200, 600/2-200);
			MoveToEx(MemDC, 800/2-200, 600/2+200, NULL);
			LineTo(MemDC, 800/2+200, 600/2+200);

			for(int i = 0; i < M; i++)
			{
				long double x = (*X)[i].d1(one);
				long double y = (*X)[i].d1(two);
				if((*Y)[i]==1)
				{
					oldPen = (HPEN)SelectObject(MemDC, data1Pen);
				}
				else
				{
					oldPen = (HPEN)SelectObject(MemDC, data2Pen);
				}
				Ellipse(MemDC, 800/2-200 + 400*x-1, 600/2+200 + 400*(-y)-1, 
								800/2-200 + 400*x+1, 600/2+200 + 400*(-y)+1);
			}

			oldPen = (HPEN)SelectObject(MemDC, linePen);
			if(-W[one]/W[two] >= 1)
			{
				MoveToEx(MemDC, 800/2-200, 600/2+200, NULL);
				LineTo(MemDC, 800/2-200 + 400*(-W[two]/W[one]), 600/2+200 + 400*(-1));
				printf("%4d %4d  %10.8Lf/%10.8Lf = %10.8Lf\n",one,two,W[one],W[two],-W[two]/W[one]);
				fflush(stdout);
			}
			else
			{
				MoveToEx(MemDC, 800/2-200, 600/2+200, NULL);
				LineTo(MemDC, 800/2-200 + 400*(1), 600/2+200 + 400*(W[one]/W[two]));
				printf("%4d %4d  %10.8Lf/%10.8Lf = %10.8Lf\n",one,two,W[one],W[two],-W[one]/W[two]);
				fflush(stdout);
			}
		}


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
						ret += Alpha[i] * Alpha[j] * (*Y)[i] * (*Y)[j] * (*kernel_matrix)[i][j];
					}
				}
			}
			L -= 0.5*ret;

			//escape condition
			if(loop > LOOP*10)
			{
				if(L - oldL <= L*relative_error)
					return true;
			}
			oldL = L;
		}
		return false;
	//}
}*/

void binSVM::calc_bias()
{
	//get Bias
	long double ret_sum = 0;
	int ret_cnt = 0;
	for(int i = 0; i < M; i++)
	{
		if (1e-5 < Alpha[i] && Alpha[i] < C - 1e-5)
		{
			long double ret = (1 - (*Y)[i] * (predict((*X)[i])-Bias)) / (*Y)[i];
			ret_sum += ret;
			ret_cnt++;
		}
	}
	printf("Bias: %Lf\n",Bias = ret_sum/(long double)ret_cnt);

	for(int i = 0; i<M; i++)
		predict_training[i] += Bias;
}

void binSVM::testing(int idx)
{
	int wrong_count = 0;
	long double error_sum = 0;
	char out_filename1[1024]={}, out_filename2[1024]={};

	sprintf(out_filename1,"training_error(%d).txt",idx);
	FILE* fp1 = fopen(out_filename1,"w");

	sprintf(out_filename2,"testing_error(%d).txt",idx);
	FILE* fp2 = fopen(out_filename2,"w");

	for(int i = 0; i<M; i++)
	{
		long double predicted = sigmoid(predict((*X)[i]))*2-1;
		long double ret;
		if((*Y)[i] >= 0)
			ret = 1;
		else
			ret = -1;
		if((ret==1 && predicted < 0.5) || (ret==-1 && predicted > -0.5))
			wrong_count++;
		error_sum += abs(predicted - ret);
		fprintf(fp1, "%.20Lf\n", abs(predicted - ret));
	}
	fclose(fp1);

	int test_wrong_count = 0;
	long double test_error_sum = 0;
	for(int i = 0; i<CLASS_NUM*TEST_NUM; i++)
	{
		long double predicted = sigmoid(predict((*T_X)[i]))*2-1;
		if(((*T_Y)[i]==1 && predicted < 0.5) || ((*T_Y)[i]==-1 && predicted > -0.5))
			test_wrong_count++;
		test_error_sum += abs(predicted - (*T_Y)[i]);
		fprintf(fp2, "%.20Lf\n", abs(predicted - (*T_Y)[i]));
	}
	fclose(fp2);

	printf("L: %Lf   wrong:%d/%d(=%Lf)  error_sum(cost):%Lf    error_avg:%Lf    /test:/  wrong:%d/%d(=%Lf)  error_sum(cost):%Lf    error_avg:%Lf\n",
		L,wrong_count,M,(long double)wrong_count/M, error_sum, error_sum/(long double)M,
		test_wrong_count,CLASS_NUM*TEST_NUM,(long double)test_wrong_count/(CLASS_NUM*TEST_NUM), test_error_sum, test_error_sum/(long double)(CLASS_NUM*TEST_NUM));
	fflush(stdout);

	sprintf(out_filename1,"alpha%d.txt",idx);
	FILE* fp = fopen(out_filename1,"w");
	for(int i=0; i<M; i++)
	{
		fprintf(fp, "%.20Lf\n",Alpha[i]);
	}
	fclose(fp);
}


long double binSVM::sigmoid(long double x)
{
	return 1/(1+pow(E,-10*x));
}