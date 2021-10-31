#include "syscall.h"

#define loops 20000

int main()
{
	int i;
	for (i = 0; i < loops; i++)
	{
		Exec("../test/adder");
		Exec("../test/matmult");
		Exec("../test/adder");
		Exec("../test/adder");
		Exec("../test/matmult");
	}
	Write("\nFinished 100000!\n", 20, ConsoleOutput);
Exit(i);
}
