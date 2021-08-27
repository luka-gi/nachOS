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
//struct for passing multiple data into thread
typedef struct threadInfo{
    int id;
    int nl;
};
//End code changes by Lucas Blanchard

//----------------------------------------------------------------------
// SimpleThread
// 	Loop 5 times, yielding the CPU to another ready thread 
//	each iteration.
//
//	"which" is simply a number identifying the thread, for debugging
//	purposes.
//----------------------------------------------------------------------

//Begin code changes by Lucas Blanchard
int num = 0;

void SimpleThread(int param){
    threadInfo* ti = (threadInfo*)param;
    for(int i = 0; i < ti->nl; i++){
        num++;
        printf("Thread %d looped %d times. num is %d\n", ti->id, i, num);
        currentThread->Yield();
    }
}
//End code changes by Lucas Blanchard


//----------------------------------------------------------------------
// ThreadTest
// 	Invoke a test routine.
//----------------------------------------------------------------------

//Begin code changes by Lucas Blanchard
void ThreadTest(){
    Thread *t1 = new Thread("T 1");
    Thread *t2 = new Thread("T 2");

    threadInfo *ti1 = new threadInfo();
    threadInfo *ti2 = new threadInfo();

    ti1->id=1;ti2->id=2;
    ti1->nl=numLoops+1;ti2->nl=numLoops;

    t1->Fork(SimpleThread, (int) ti1);
    t2->Fork(SimpleThread, (int) ti2);
}
//End code changes by Lucas Blanchard
