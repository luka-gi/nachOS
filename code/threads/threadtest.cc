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

    //only break if input is valid (or only loops if blank input)
    bool validEntry = false;
    while (!validEntry)
    {
        if (inputOverflow(input))
        {
            printf("input overflow, try again: ");
            fgets(input, arrSize, stdin);
        }
        //find the category that the input belongs to
        else if (isInteger(input))
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
        //single character that somehow isn't newline OR single character and newline
        else if ((strlen(input) == 1 && input[0] != '\n') || (strlen(input) == 2 && input[1] == '\n'))
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
        //blank input, so take a new input
        else
        {
            printf("Input was blank, try again: ");
            fgets(input, arrSize, stdin);
        }
    }
}

void shoutingThreads(int param)
{
    return;
}

//determine if input char array resembles an integer
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

//determine if input char array resembles a decimal
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

//determine if char array resembles negative integer
bool isNegInt(char *input)
{
    //starts with '-' and following characters are int == negative int
    if (input[0] == '-' && isInteger(input + 1))
    {
        return true;
    }
    return false;
}

//determine if char array resembles negative decimal
bool isNegDec(char *input)
{
    //starts with '-' and following characters are dec == negative dec
    if (input[0] == '-' && isDecimal(input + 1))
    {
        return true;
    }
    return false;
}

//pass in string taken from fgets.
//fgets only reads from stdin if the char array is not overfilled.
//in the case of overflow, stdin still has characters that need to be read
//this function flushes stdin and returns that it has detected overflow
bool inputOverflow(char *input)
{
    if (input[strlen(input)] == '\0' && input[strlen(input) - 1] != '\n')
    {
        int charFromInput;
        while ((charFromInput = getchar() != '\n' && charFromInput != EOF))
        {
            //this while loop, the getchar functin removes stdin chars from the buffer
        }
        return true;
    }
    return false;
}

//ThreadTest() run by nachos
void ThreadTest()
{
    if (projTask == 1)
    {
        Thread *inputIdent = new Thread("Project1_input_identification");
        inputIdent->Fork(inputIdentification, 0);
    }
    else if (projTask == 2)
    {
        //5 bytes can hold 0-999 from user input, should be fine for both vals
        //(1 byte will be passed a newline)
        int arrSize = 5;
        char *numThreadsInput = new char[arrSize];
        char *numShoutsInput = new char[arrSize];
        int numThreads;
        int numShouts;

        //prompt and capture valid input
        bool validInput = false;

        //numThreads input validation
        printf("enter number of threads to shout (from 0-999): ");
        fgets(numThreadsInput, arrSize, stdin);

        while (!validInput)
        {
            if (inputOverflow(numThreadsInput))
            {
                printf("input overflow error. enter number of threads to shout (from 0-999): ");
                fgets(numThreadsInput, arrSize, stdin);
            }
            else if (!isInteger(numThreadsInput))
            {
                printf("that was not an integer. enter number of threads to shout (from 0-999): ");
                fgets(numThreadsInput, arrSize, stdin);
            }
            else
            {
                validInput = true;
                numThreads = atoi(numThreadsInput);
            }
        }

        //reset this to reuse it
        validInput = false;

        //numShouts input validation
        printf("enter number of shouts per thread (from 0-999): ");
        fgets(numShoutsInput, arrSize, stdin);

        while (!validInput)
        {
            if (inputOverflow(numShoutsInput))
            {
                printf("input overflow error. enter number of threads to shout (from 0-999): ");
                fgets(numShoutsInput, arrSize, stdin);
            }
            else if (!isInteger(numShoutsInput))
            {
                printf("that was not valid input. enter number of shouts per thread (from 0-999): ");
                fgets(numShoutsInput, arrSize, stdin);
            }
            else
            {
                validInput = true;
                numShouts = atoi(numShoutsInput);
            }
        }

        printf("\nboth inputs valid, forking threads\n\n");

        for (int i = 0; i < numThreads; i++)
        {
            char buf[20];
            snprintf(buf, sizeof buf, "Shouting Thread %d", i);
            Thread *t = new Thread(buf);
            t->Fork(shoutingThreads, i);
        }
    }
    //yield and end main thread once forked, or once illegal projTask value is passed
    currentThread->Finish();
}
//End code changes by Lucas Blanchard