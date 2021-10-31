#include "syscall.h"

int main()
{
	Write("\nExec Adder 1\n", 20, ConsoleOutput);
	Exec("../test/adder");
	Write("\nExec Adder 2\n", 20, ConsoleOutput);
	Exec("../test/adder");
	Write("\nExec Matmult 1\n", 20, ConsoleOutput);
	Exec("../test/matmult");
	Write("\nExec Sort 1\n", 20, ConsoleOutput);
	Exec("../test/sort");
	Write("\nYield\n", 20, ConsoleOutput);
	Yield();
	Yield();
	Yield();
	Yield();
	Yield();
	Yield();
	Yield();
	Yield();
	Yield();
	Yield();
	Yield();
	Yield();
	Write("\nJoin Matmult 2\n", 20, ConsoleOutput);
	Join(Exec("../test/matmult"));
	Write("\nExec Matmult 3\n", 20, ConsoleOutput);
	Exec("../test/matmult");
	Write("\nJoin Sort 2\n", 20, ConsoleOutput);
	Join(Exec("../test/sort"));
	Write("\nJoin Adder 3", 20, ConsoleOutput);
	Join(Exec("../test/adder"));
	Write("\nJoin Adder 4", 20, ConsoleOutput);
	Join(Exec("../test/adder"));
	// Yield();
	// Write("\nJoin Adder 3\n", 20, ConsoleOutput);
	// Join(Exec("../test/adder"));
	// Write("\nExec Adder 4\n", 20, ConsoleOutput);
	// Exec("../test/adder");
	Write("\nCOMMENTED HALTING!!!\n", 20, ConsoleOutput);
	//may accidentally yield and call halt before finishing other processes
	//Exec("../test/halt");
}
