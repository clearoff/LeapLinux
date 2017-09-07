#include <iostream>
using namespace std;

typedef struct Atest{
	int a;
	int b;
}Atest;

int main()
{
	Atest a;
	Atest& b = a;
	cout<<"a:"<<sizeof(a)<<endl;
	cout<<"a&:"<<sizeof(b)<<endl;
	return 0;
}
