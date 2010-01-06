/* Debug function
 * Used to define a global debug level and print evry call to printf_dpg with a level < to the global debug level.
 */
#include <stdio.h>
#include <stdarg.h>


int g_level=1;	//Global variable


void printf_dbg(int level, const char* szFormat, ...) {
	if(level <= g_level) {
	//	printf("DEBUG(%i) : ",level);
		va_list args;
		va_start(args,szFormat);
		vfprintf(stdout,szFormat,args);
		fflush(stdout);
		va_end(args);
	}
}
