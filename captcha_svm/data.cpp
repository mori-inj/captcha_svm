#include "data.h"

Data::Data(int y, int x)
{
	n = y*x;
	height = y, width = x;
	data = vector < vector<long double> >(y);
	for(int i = 0; i < y; i++)
	{
		data[i] = vector<long double> (x);
	}
}