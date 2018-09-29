#include <stdio.h>
#include <stddef.h>
#include <stdarg.h>

#define ERROR_LIST_MAX		20
#define ERROR_LENGTH_MAX	100

static char error_stack[ERROR_LIST_MAX][ERROR_LENGTH_MAX];
static int N = 0;

void error_log(char *error, ...)
{
	va_list argList;

	if (N < ERROR_LIST_MAX)
	{
		va_start(argList, error);
		vsnprintf(error_stack[N++], ERROR_LENGTH_MAX - 1, error, argList);
		va_end(argList);
	}
}

char *error_get(void)
{
	if (N > 0)
	{
		return error_stack[--N];
	}
	else
	{
		return NULL;
	}
}