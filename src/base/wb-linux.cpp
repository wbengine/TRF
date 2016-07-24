#include "wb-linux.h"
#ifdef __linux
namespace wb
{
	const char *wday[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
	void _strdate(char *str)
	{
		time_t timep;
		struct tm *p;
		time(&timep);
		p = localtime(&timep);
		sprintf(str, "%d%d%d %s", (1900+p->tm_yday), (1+p->tm_mon), p->tm_mday, wday[p->tm_wday]);
	}
	void _strtime(char *str)
	{
		time_t timep;
		struct tm *p;
		time(&timep);
		p = localtime(&timep);
		sprintf(str, "%d:%d:%d", p->tm_hour, p->tm_min, p->tm_sec);
	}
	void getch()
	{
	}

}
#endif
