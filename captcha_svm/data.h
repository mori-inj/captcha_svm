#ifndef __DATA__
#define __DATA__

#include <vector>

using namespace std;

class Data
{
private:
	int width, height;
	vector< vector<long double> > data;
public:
	int n;
	Data() {};
	Data(int y, int x);
	void set(int i, int j, long double d) { data[i][j] = d; }
	long double d1(int idx){ return data[idx/width][idx%width]; }
	long double d2(int i, int j) { return data[i][j]; }
};

#endif