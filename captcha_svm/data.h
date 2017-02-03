#ifndef __DATA__
#define __DATA__

#include <vector>

using namespace std;

class Data
{
private:
	int size;
	vector< long double > data;
public:
	Data() {};
	Data(int n);
	void set(int i, long double d) { data[i] = d; }
	long double d1(int idx){ return data[idx]; }
};

#endif