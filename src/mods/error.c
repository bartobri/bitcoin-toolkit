#include <stddef.h>

#define ERROR_MAX 50

static char *error_stack[ERROR_MAX];
static int N = 0;

void error_push(char *error)
{
	if (N < ERROR_MAX)
	{
		error_stack[N++] = error;
	}
}

char *error_pop(void)
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