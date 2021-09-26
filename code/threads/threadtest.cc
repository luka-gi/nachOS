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

//Begin proj2 changes by Lucas Blanchard
//include for semaphore code
#include "synch.h"
//global variable for use in shoutingThreads task, moved here from system.h
int numShouts;

//for task 1
int M;
int P;
int S;
bool allSat = 0;
int notFinished = 0; //num of philos not finished last meal
bool *chopSticks;
//protects shared counting variables (for busy waiting)
bool MAvail = 1;
bool NFAvail = 1;
//for task 2
Semaphore *mealSem = new Semaphore("Meal Semaphore", 1);
Semaphore *NFSem = new Semaphore("Unfinished Threadcount Semaphore", 1);
Semaphore **chopStickSem;
Semaphore *allSatSem = new Semaphore("Thread Startblock Semaphore", 0);
Semaphore *allLeave = new Semaphore("Thread Deathblock Semaphore", 0);
//End proj2 changes by Lucas Blanchard

//Begin proj1 code changes by Lucas Blanchard

void inputIdentification(int which)
{
    //initialize arbitrary sized array for input storage
    int arrSize = 1024;
    char *input = new char[arrSize];

    //prompt and capture input
    printf("enter something with a max byte size of %d, whitespaces don't count: ", arrSize);
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
            printf("Input was invalid, try again: ");
            fgets(input, arrSize, stdin);
        }
    }
}

void shoutingThreads(int which)
{
    //numShouts is a global variable that will NOT be changed after validation
    //should not cause race condition
    for (int i = 0; i < numShouts; i++)
    {
        int numYields = 3 + Random() % 4;
        int chooseShout = Random() % 6;

        if (chooseShout == 0)
        {
            printf("%s: For this task, you will implement a shouting match between threads.\n", currentThread->getName());
        }
        else if (chooseShout == 1)
        {
            printf("%s: Prompt the user for the number of threads, T, and the number of times each thread should shout, S, for a total of T * S shouts.\n", currentThread->getName());
        }
        else if (chooseShout == 2)
        {
            printf("%s: Each thread must be identified by a unique ID number when shouting.\n", currentThread->getName());
        }
        else if (chooseShout == 3)
        {
            printf("%s: After prompting the user for T and S, fork T threads to a shouting function.\n", currentThread->getName());
        }
        else if (chooseShout == 4)
        {
            printf("%s: Shouting consists of displaying a random phrase as output.\n", currentThread->getName());
        }
        else if (chooseShout == 5)
        {
            printf("%s: Whenever a thread shouts, it enters a busy waiting loop where it yields the CPU for 3-6 cycles randomly.\n", currentThread->getName());
        }

        for (int j = 0; j < numYields; j++)
        {
            currentThread->Yield();
        }
    }
}

//determine if input char array resembles an integer
bool isInteger(char *input)
{
    //loop through array to determine if is int or not
    for (int i = 0; i < strlen(input); i++)
    {

        //if input is ONLY a newline or null, then is empty and non-number
        if (input[i] == '\n' && strlen(input) <= 1)
        {
            return false;
        }
        //if char is not positive digit or any newline (targtting the one with fgets), leave loop as number is not int
        else if ((atoi(&input[i]) <= 0 && input[i] != '0') && (input[i] != '\n'))
        {
            return false;
        }
        //otherwise if detected final newline from fgets other type of whitespace, do not consider int
        else if ((input[i] == '\n' && i != strlen(input) - 1) || input[i] == ' ' || input[i] == '\t' || input[i] == '\v' || input[i] == '\f' || input[i] == '\r')
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

//Begin proj2 code changes by Lucas Blanchard

//test functions which acts as CPU yields
void randYield()
{
    if ((Random() % 5) == 0)
    {
        currentThread->Yield();
    }
}

void busyWait(int min, int max)
{
    int numYields = min + Random() % (max - min + 1);
    for (int i = 0; i < numYields; i++)
    {
        currentThread->Yield();
    }
}

//task 1 philosopher code
void busyPhilos(int which)
{
    //busy waiting loop until ALL threads are created
    while (!allSat)
    {
        currentThread->Yield();
    }

    //while there are meals available
    while (M)
    {
        //make sure we have both chopsticks
        int chopCount;
        while (chopCount != 2)
        {
            chopCount = 0;
            //wait until left chopstick available
            while (!chopSticks[which])
            {
                currentThread->Yield();
            }
            //pick up chopstick, mark it unavailable, inc chopCount
            chopSticks[which] = 0;
            chopCount++;
            printf("\n-Philosopher %d has picked up left chopstick", which);

            //attempt maxtrials amount of times to pick up the right chopstick
            //we know eating can take about 7 cycles, so if it lasts any longer than this then it is likely deadlocked
            int maxTrials = 7;
            int countTrials = 0;
            for (int i = 0; i < maxTrials; i++)
            {
                //if right chopstick unavailable, yield and wait until the for loop is over
                if (!chopSticks[(which + 1) % P])
                {
                    countTrials++;
                    currentThread->Yield();
                }
                else
                {
                    //chopstick IS available
                    //pick up chopstick, mark it unavailable, inc chopCount, exit the loop
                    chopSticks[(which + 1) % P] = 0;
                    printf("\n--Philosopher %d has picked up right chopstick", which);
                    chopCount++;
                    break;
                }
            }

            //if numTrials did not break, then we did not pick up right chopstick. countTrials did not reach maxTrials. therefore give up left chopstick
            //we will restart this while loop because chopCount did not get incremented to 2
            //yield a random number of times as in network collision prevention
            //prevents this thread from immediately accessing the same data, thus causing another deadlock
            if (countTrials == maxTrials)
            {
                chopSticks[which] = 1;
                printf("\n----To prevent deadlock, philosopher %d politely dropped left chopstick", which);
                busyWait(0, 10);
            }
        }
        //reset for the next for loop
        chopCount = 0;

        //if there is still a meal despite random thread yields, then we are allowed to eat
        if (M)
        {
            //wait for M to finish being modified
            while (!MAvail)
            {
                currentThread->Yield();
            }
            //make M unavailale, decrement, then make M available
            MAvail = 0;
            M--;
            printf("\n---Philosopher %d has begun eating. Meals remaining: %d", which, M);
            MAvail = 1;
            //complete the actual eating
            busyWait(3, 6);
        }

        //drop both chopsticks
        printf("\n----Philosopher %d has dropped left chopstick", which);
        chopSticks[which] = 1;
        printf("\n-----Philosopher %d has dropped right chopstick", which);
        chopSticks[(which + 1) % P] = 1;

        //think after eating
        printf("\n------Philosopher %d has begun thinking", which);
        busyWait(3, 6);
    }

    //wait for notFinished to finish being modified
    //each thread may only do this once to signify that they are waiting for all threads to finish
    printf("\n-------Philosopher %d has finished their last meal", which);
    while (!NFAvail)
    {
        currentThread->Yield();
    }
    NFAvail = 0;
    notFinished--;
    //if this decrement means notFinished == 0, then this is the last thread
    if (notFinished == 0)
    {
        printf("\n\n-----------------All philosophers will now leave the table------------------\n\n");
    }
    NFAvail = 1;

    //wait indefinitely until final thread is ready to die
    while (true)
    {
        //doesn't matter if this is shared since we're only trying to read 0 or nonzero.
        if (notFinished == 0)
        {
            break;
        }
        currentThread->Yield();
    }
    printf("--------Philosopher %d is exiting\n", which);
}

//task 2 philospher code
void semPhilos(int which)
{
    //wait for all threads to be forked first
    allSatSem->P();
    allSatSem->V();

    //loop to break out of once M = 0. avoids reading shared resource M.
    while (true)
    {
        chopStickSem[which]->P();
        printf("\n-Philosopher %d has picked up left chopstick", which);
        chopStickSem[(which + 1) % P]->P();
        printf("\n--Philosopher %d has picked up right chopstick", which);

        //hoard M for reading and writing
        mealSem->P();
        //there are still meals left
        if (M != 0)
        {
            M--;
            printf("\n---Philosopher %d has begun eating. Meals remaining: %d", which, M);
            //release M once finished
            mealSem->V();
            //eat
            busyWait(3, 6);

            //drop chopsticks
            chopStickSem[which]->V();
            printf("\n----Philosopher %d has dropped left chopstick", which);
            chopStickSem[(which + 1) % P]->V();
            printf("\n-----Philosopher %d has dropped right chopstick", which);

            printf("\n------Philosopher %d has begun thinking", which);
            busyWait(3, 6);
        }
        //there are no longer any meals, we can leave the loop
        else
        {
            mealSem->V();
            break;
        }
    }
    //drop chopsticks (since the last iteration doesn't enter the first if condition)
    chopStickSem[which]->V();
    printf("\n----Philosopher %d has dropped left chopstick", which);
    chopStickSem[(which + 1) % P]->V();
    printf("\n-----Philosopher %d has dropped right chopstick", which);
    printf("\n-------Philosopher %d has finished their last meal", which);

    //NF starts from P (num of threads) and is decreased once per thread
    NFSem->P();
    notFinished--;
    //this following statement will only be true for the last thread
    if (notFinished == 0)
    {
        printf("\n\n-----------------All philosophers will now leave the table------------------\n\n");
        allLeave->V();
    }
    NFSem->V();
    allLeave->P();
    allLeave->V();
    printf("--------Philosopher %d is exiting\n", which);
}
//End proj2 code changes by Lucas Blanchard

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

        //prompt and capture valid input
        bool validInput = false;

        //numThreads input validation
        printf("enter number of threads to shout (from 0-999), whitespaces don't count: ");
        fgets(numThreadsInput, arrSize, stdin);

        while (!validInput)
        {
            //input size > max size is invalid
            if (inputOverflow(numThreadsInput))
            {
                printf("input overflow error. enter number of threads to shout (from 0-999): ");
                fgets(numThreadsInput, arrSize, stdin);
            }
            //only integers are valid
            else if (!isInteger(numThreadsInput))
            {
                printf("that was not an integer. enter number of threads to shout (from 0-999): ");
                fgets(numThreadsInput, arrSize, stdin);
            }
            //input is valid, accept input
            else
            {
                validInput = true;
                numThreads = atoi(numThreadsInput);
            }
        }

        //reset this to reuse it
        validInput = false;

        //numShouts input validation
        printf("enter number of shouts per thread (from 0-999), whitespaces don't count: ");
        fgets(numShoutsInput, arrSize, stdin);

        //same as previous loop
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

        DEBUG('t', "\nboth inputs valid, forking threads\n\n");

        //creates user specified number of threads
        for (int i = 0; i < numThreads; i++)
        {
            int threadNameMaxLen = strlen("Shouting Thread 999");
            char *buf = new char[threadNameMaxLen];
            snprintf(buf, threadNameMaxLen, "Shouting Thread %d", i);

            Thread *t = new Thread(buf);
            t->Fork(shoutingThreads, i);
        }
    }
    //Begin proj2 code changes by Lucas Blanchard
    else if (projTask == 3)
    {
        //prompt and capture valid input
        //arrSize input of 7 should be plenty (99999) to support 10k
        int arrSize = 7;
        char *MInput = new char[arrSize];
        char *PInput = new char[arrSize];

        bool validInput = false;

        //M validation
        printf("enter number of meals to be eaten, whitespaces don't count and input bytesize must be %d or less: ", arrSize);
        fgets(MInput, arrSize, stdin);

        while (!validInput)
        {
            //input size > max size is invalid
            if (inputOverflow(MInput))
            {
                printf("input overflow error. enter number of meals to be eaten (size %d): ", arrSize);
                fgets(MInput, arrSize, stdin);
            }
            //only integers are valid
            else if (!isInteger(MInput))
            {
                printf("that was not an integer. enter number of meals to be eaten (size %d): ", arrSize);
                fgets(MInput, arrSize, stdin);
            }
            //input is valid, accept input
            else
            {
                validInput = true;
                M = atoi(MInput);
            }
        }

        //P validation
        validInput = false;
        printf("enter number of philosophers to dine, whitespaces don't count and input bytesize must be %d or less: ", arrSize);
        fgets(PInput, arrSize, stdin);

        while (!validInput)
        {
            //input size > max size is invalid
            if (inputOverflow(PInput))
            {
                printf("input overflow error. enter number of philosophers to dine (size %d): ", arrSize);
                fgets(PInput, arrSize, stdin);
            }
            //only integers are valid
            else if (!isInteger(PInput))
            {
                printf("that was not an integer. enter number of philosophers to dine (size %d): ", arrSize);
                fgets(PInput, arrSize, stdin);
            }
            //if P = 1, there is only one chopstick to eat a meal
            else if (atoi(PInput) == 1)
            {
                printf("P cannot be equal to 1. enter number of philosophers to dine (size %d): ", arrSize);
                fgets(PInput, arrSize, stdin);
            }
            //input is valid, accept input
            else
            {
                validInput = true;
                P = atoi(PInput);
                //since P has been successfully verified, give chopstick array a size of number of philosophers
                chopSticks = new bool[P];
                //initialize all chopsticks to 1. this is more clear than assigning 0 to mean "available"
                for (int i = 0; i < P; i++)
                {
                    chopSticks[i] = 1;
                }
                notFinished = P;
            }
        }

        //input validation complete
        for (int i = 0; i < P; i++)
        {
            int threadNameMaxLen = strlen("Philosopher Thread 1000000");
            char *buf = new char[threadNameMaxLen];
            snprintf(buf, threadNameMaxLen, "Philosopher Thread %d", i);

            Thread *t = new Thread(buf);
            t->Fork(busyPhilos, i);
            printf("\nPhilosopher %d is waiting to be seated", i);
        }
        //all threads have been forked, therefore they may sit
        printf("\n\n----------------------All philosophers have been seated---------------------\n");
        allSat = 1;
    }
    else if (projTask == 4)
    {
        //prompt and capture valid input
        //arrSize input of 7 should be plenty (99999) to support 10k
        int arrSize = 7;
        char *MInput = new char[arrSize];
        char *PInput = new char[arrSize];

        bool validInput = false;

        //M validation
        printf("enter number of meals to be eaten, whitespaces don't count and input bytesize must be %d or less: ", arrSize);
        fgets(MInput, arrSize, stdin);

        while (!validInput)
        {
            //input size > max size is invalid
            if (inputOverflow(MInput))
            {
                printf("input overflow error. enter number of meals to be eaten (size %d): ", arrSize);
                fgets(MInput, arrSize, stdin);
            }
            //only integers are valid
            else if (!isInteger(MInput))
            {
                printf("that was not an integer. enter number of meals to be eaten (size %d): ", arrSize);
                fgets(MInput, arrSize, stdin);
            }
            //input is valid, accept input
            else
            {
                validInput = true;
                M = atoi(MInput);
            }
        }

        //P validation
        validInput = false;
        printf("enter number of philosophers to dine, whitespaces don't count and input bytesize must be %d or less: ", arrSize);
        fgets(PInput, arrSize, stdin);

        while (!validInput)
        {
            //input size > max size is invalid
            if (inputOverflow(PInput))
            {
                printf("input overflow error. enter number of philosophers to dine (size %d): ", arrSize);
                fgets(PInput, arrSize, stdin);
            }
            //only integers are valid
            else if (!isInteger(PInput))
            {
                printf("that was not an integer. enter number of philosophers to dine (size %d): ", arrSize);
                fgets(PInput, arrSize, stdin);
            }
            //if P = 1, there is only one chopstick to eat a meal
            else if (atoi(PInput) == 1)
            {
                printf("P cannot be equal to 1. enter number of philosophers to dine (size %d): ", arrSize);
                fgets(PInput, arrSize, stdin);
            }
            //input is valid, accept input
            else
            {
                validInput = true;
                P = atoi(PInput);
                notFinished = P;

                //initialize chopsticks to place in our array
                chopStickSem = new Semaphore *[P];
                int threadNameMaxLen = strlen("Chopstick Semaphore 99999");
                char *buf = new char[threadNameMaxLen];
                for (int i = 0; i < P; i++)
                {
                    snprintf(buf, threadNameMaxLen, "Chopstick Semaphore %d", i);
                    chopStickSem[i] = new Semaphore(buf, 1);
                }
            }
        }

        //input validation complete
        for (int i = 0; i < P; i++)
        {
            int threadNameMaxLen = strlen("Philosopher Thread 99999");
            char *buf = new char[threadNameMaxLen];
            snprintf(buf, threadNameMaxLen, "Philosopher Thread %d", i);

            Thread *t = new Thread(buf);
            t->Fork(semPhilos, i);
            printf("\nPhilosopher %d is waiting to be seated", i);
        }
        //all threads have been forked, therefore they may sit
        printf("\n\n----------------------All philosophers have been seated---------------------\n");
        //signal to all threads to release their block
        allSatSem->V();
    }
    else if (projTask == 5)
    {
        }
    else if (projTask == 6)
    {
    }
    else if (projTask == 0)
    {
    }
    //End proj2 code changes by Lucas Blanchard

    //yield and end main thread once forked, or once illegal projTask value is passed
    currentThread->Finish();
}
//End proj1 code changes by Lucas Blanchard