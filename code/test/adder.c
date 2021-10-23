#include "syscall.h"

#define loops 2000

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
		if (1999 == i)
		{
			Write("\n10th loop!\n\n", 20, ConsoleOutput);
		}
	}
	Write("\nAdding Complete!\n", 20, ConsoleOutput);
	Exit(0);
}
