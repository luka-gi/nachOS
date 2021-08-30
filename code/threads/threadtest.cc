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

    for (num = 0; num < 5; num++)
    {
        printf("*** thread %d looped %d times\n", param, num);
        currentThread->Yield();
    }
}

//Begin code changes by Lucas Blanchard
void PingPong(int loops)
{
    int num;

    for (num = 1; num <= loops; num++)
    {
        if (num % 2 != 0)
        {
            printf("%s: \"Ping\" looped %d times\n", currentThread->getName(), num);
        }
        else
        {
            printf("%s: \"Pong\" looped %d times\n", currentThread->getName(), num);
        }
        currentThread->Yield();
    }
}

void HelloWorld(int loops)
{
    int num;

    for (num = 1; num <= loops; num++)
    {
        printf("%s: \"Hello World!\" looped %d times\n", currentThread->getName(), num);
        currentThread->Yield();
    }
}

void CountUp(int loops)
{
    int num;

    for (num = 1; num <= loops; num++)
    {
        printf("%s: \"Count to %d: %d\"\n", currentThread->getName(), loops, num);
        currentThread->Yield();
    }
}
//End code changes by Lucas Blanchard

//----------------------------------------------------------------------
// ThreadTest
// 	Invoke a test routine.
//----------------------------------------------------------------------

//Begin code changes by Lucas Blanchard
void ThreadTest()
{
    Thread *t1, *t2, *t3;

    t1 = new Thread("T_1");
    t2 = new Thread("T_2");
    t3 = new Thread("T_3");

    t1->Fork(HelloWorld, 3);
    t2->Fork(PingPong, 4);
    t3->Fork(CountUp, 5);
}
//End code changes by Lucas Blanchard