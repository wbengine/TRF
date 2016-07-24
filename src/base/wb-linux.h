#pragma once
#ifdef __linux
#include <iostream>
#include <ctime>
#include <cstring>
using namespace std;
#include <stdio.h>
#include <stdlib.h>

#define _ftelli64 ftello64
#define _fseeki64 fseeko64
#define _vsnprintf vsnprintf

namespace wb
{
	void _strdate(char *str);
	void _strtime(char *str);
	void getch();
}
#endif //__linux
