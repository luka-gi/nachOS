#include "syscall.h"

int main()
{
	Write("\nExec Adder 1\n", 20, ConsoleOutput);
	Exec("../test/adder");
	Yield();
	Write("\nExec Adder 2\n", 20, ConsoleOutput);
	Exec("../test/adder");
	Write("\nExec Matmult 1\n", 20, ConsoleOutput);
	Exec("../test/matmult");
	Write("\nExec Sort\n", 20, ConsoleOutput);
	Exec("../test/sort");
	Write("\nJoin Adder 3\n", 20, ConsoleOutput);
	Join(Exec("../test/adder"));
	Join(Exec("../test/adder"));
	Join(Exec("../test/adder"));
	Write("\nExec Adder 4\n", 20, ConsoleOutput);
	Exec("../test/adder");
	Write("\nHALTING!!!\n", 20, ConsoleOutput);
	Exec("../test/halt");
}
