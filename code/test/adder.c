#include "syscall.h"

#define loops 10

int main()
{
	int i, x;
	x = 0;
	for (i = 0; i < loops; i++)
	{
		x = x + i;
		if (0 == i)
		{
			Write("\n1st loop!\n\n", 20, ConsoleOutput);
		}
		if (9 == i)
		{
			Write("\n10th loop!\n\n", 20, ConsoleOutput);
		}
	}
	Write("\nAdding Complete1!1\n", 20, ConsoleOutput);
Exit(x);
}
