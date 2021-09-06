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
void inputIdentification(int param)
{
    //initialize arbitrary sized array for input storage
    int arrSize = 1024;
    char *input = new char[arrSize];

    //prompt and capture input
    printf("enter something with a max size of %d: \n", arrSize);
    fgets(input, arrSize, stdin);

    ////////////////////////////////
    //printf("\nyour input %s is int?: %d\n", strtok(input, "\n"), isInteger(input));
    //printf("\nyour input %s is dec?: %d\n", strtok(input, "\n"), isDecimal(input));
    //printf("\nyour input %s is negint?: %d\n", strtok(input, "\n"), isNegInt(input));
    printf("\nyour input %s is negdec?: %d\n", strtok(input, "\n"), isNegDec(input));
    ////////////////////////////////
    //int
    //dec = int.int
    //neg = "-"dec or "-"int
    //char
    //char str
    ///////////////////////////////
}

bool isInteger(char *input)
{
    //loop through array to determine if is int or not
    for (int i = 0; i < strlen(input); i++)
    {
        //if char is not digit, leave loop as number is not int
        //if nondigit and nonnewline, must be nonnumber
        if (atoi(&input[i]) <= 0 && input[i] != '\n' && input[i] != '0')
        {
            return false;
        }
        //otherwise, if input is ONLY a newline, still a nonnumber
        else if (input[i] == '\n' && strlen(input) == 1)
        {
            return false;
        }
    }
    //if not the other cases throughout entire array, must be int
    return true;
}

bool isDecimal(char *input)
{
    //initiate the decimal pointer as null
    char *decPointer = NULL;

    for (int i = 0; i < strlen(input); i++)
    {
        //on the first instance of a decimal point, split string
        if (input[i] == '.')
        {
            //decimal point found, assign its address to decpointer
            decPointer = &input[i];
            //if decpointer address equals input address, then the integer part is null, therefore not a decimal number
            if (decPointer != input)
            {
                //we turn the decimal point into null terminator to treat substrings as SEPARATE strings
                *decPointer = '\0';
                //if the subtrings on both sides of the decimal are integers, then input is the decimal
                if (isInteger(input) && isInteger(decPointer + 1))
                {
                    //change null terminator back to decimal point to avoid errors
                    *decPointer = '.';
                    return true;
                }
                //change null terminator back to decimal point to avoid errors
                *decPointer = '.';
            }
            //break on the first sighting of a decimal
            //if there are multiple decimal points, it is not a decimal
            break;
        }
    }
    return false;
}

bool isNegInt(char *input)
{
    //starts with '-' and following characters are int == negative int
    if (input[0] == '-' && isInteger(input + 1))
    {
        return true;
    }
    return false;
}

bool isNegDec(char *input)
{
    //starts with '-' and following characters are dec == negative dec
    if (input[0] == '-' && isDecimal(input + 1))
    {
        return true;
    }
    return false;
}
//End code changes by Lucas Blanchard

//----------------------------------------------------------------------
// ThreadTest
// 	Invoke a test routine.
//----------------------------------------------------------------------

//Begin code changes by Lucas Blanchard
void ThreadTest()
{
    Thread *inputIdent = new Thread("input_identification_thread");
    inputIdent->Fork(inputIdentification, 0);
    currentThread->Finish();
}
//End code changes by Lucas Blanchard