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
    printf("enter something with a max byte size of %d: ", arrSize);
    //fgets on a large array will be able to handle large inputs, and in the extreme case of overflow will truncate
    fgets(input, arrSize, stdin);

    bool validEntry = false;
    while (!validEntry)
    {
        if (isInteger(input))
        {
            printf("\nYour input is of the category: Nonnegative Integer Number\n\n");
            validEntry = true;
        }
        else if (isDecimal(input))
        {
            printf("\nYour input is of the category: Nonnegative Decimal Number\n\n");
            validEntry = true;
        }
        else if (isNegInt(input))
        {
            printf("\nYour input is of the category: Negative Integer Number\n\n");
            validEntry = true;
        }
        else if (isNegDec(input))
        {
            printf("\nYour input is of the category: Negative Decimal Number\n\n");
            validEntry = true;
        }
        //single character + newline
        else if (strlen(input) == 2)
        {
            printf("\nYour input is of the category: Character\n\n");
            validEntry = true;
        }
        //if not a number, not a char, must be a character string. otherwise it's blank input
        else if (strlen(input) != 1)
        {
            printf("\nYour input is of the category: Character String\n\n");
            validEntry = true;
        }
        else
        {
            printf("Input was blank, try again: ");
            fgets(input, arrSize, stdin);
        }
    }
}

bool isInteger(char *input)
{
    //loop through array to determine if is int or not
    for (int i = 0; i < strlen(input); i++)
    {
        //if char is not digit, leave loop as number is not int
        //if non-digit and non-newline, must be non-number
        if (atoi(&input[i]) <= 0 && input[i] != '\n' && input[i] != '0')
        {
            return false;
        }
        //otherwise, if input is ONLY a newline, still a non-number
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
            //if there are multiple decimal points, it is not a decimal, therefore doesn't matter
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
    if (projTask == 1)
    {
        Thread *inputIdent = new Thread("Project1_input_identification");
        inputIdent->Fork(inputIdentification, 0);
    }
    else if (projTask == 2)
    {
        //taskThread->Fork(shoutingThreads, 0);
    }
    //exit main once forked, or once illegal projTask value is passed
    currentThread->Finish();
}
//End code changes by Lucas Blanchard