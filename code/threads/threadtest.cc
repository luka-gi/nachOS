// threadtest.cc
//	Simple test case for the threads assignment.
//
//	Create two threads, and have them context switch
//	back and forth between themselves by calling Thread::Yield,
//	to illustratethe inner workings of the thread system.
//
// Copyright (c) 1992-1993,2021 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"

void SimpleThread(int param)
{
    int num;

    for (num = 0; num < param; num++)
    {
        printf("*** thread %d looped %d times\n", param, num);
        currentThread->Yield();
    }
}

//Begin code changes by Lucas Blanchard

//End code changes by Lucas Blanchard

//----------------------------------------------------------------------
// ThreadTest
// 	Invoke a test routine.
//----------------------------------------------------------------------

//Begin code changes by Lucas Blanchard
void ThreadTest()
{
    Thread *t1 = new Thread("T_1");
    if (projTask == 1)
    {
        t1->Fork(SimpleThread, 1);
    }
    else if (projTask == 2)
    {
        t1->Fork(SimpleThread, 4);
    }
    currentThread->Finish();
}
//End code changes by Lucas Blanchard