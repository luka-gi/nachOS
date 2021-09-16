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

//Begin code changes by Lucas Blanchard

//include nachos built-in semaphores
#include "synch.h"

//shared global variable count
int count;

//binary semaphore to protect count
Semaphore *S = new Semaphore("Count Semaphore", 1);

//randomly yields thread 1/5 times
void randYield()
{
    if ((Random() % 5) == 0)
    {
        currentThread->Yield();
    }
}

void semTest(int which)
{
    for (int i = 0; i < 12; i++)
    {
        S->P();
        //critical section
        int x = count;
        x++;

        //test function which acts as random CPU yield
        randYield();

        count = x;

        //end critical section
        printf("Thread %d looped %d times, count is %d\n", which, i, count);
        S->V();

        currentThread->Yield();
    }
}

//ThreadTest() run by nachos
void ThreadTest()
{
    Thread *t1, *t2;

    t1 = new Thread("T0");
    t2 = new Thread("T1");

    t1->Fork(semTest, 0);
    t2->Fork(semTest, 1);

    currentThread->Finish();
}
//End code changes by Lucas Blanchard