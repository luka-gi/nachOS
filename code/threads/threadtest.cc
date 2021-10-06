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

//for various tasks
int M;
int P;
int S;
//for task 1
bool allSat = 0;
int notFinished = 0; //num of philos who have not finished last meal
bool *chopSticks;
//protects shared counting variables (for busy waiting)
bool MAvail = 1;
bool NFAvail = 1;
//for task 2
Semaphore *mealSem = new Semaphore("Meal Semaphore", 1);
Semaphore *NFSem = new Semaphore("Unfinished Threadcount Semaphore", 1);
Semaphore **chopSticksSem;
Semaphore *allSatSem = new Semaphore("Thread Startblock Semaphore", 0);
Semaphore *allLeaveSem = new Semaphore("Thread Deathblock Semaphore", 0);
//for task 3
struct Message
{
    int sender;
    char *contents;
};
Message ***mailboxes;

Semaphore *MessageSemaphore = new Semaphore("Messages Until Simulation End Semaphore", 1);
Semaphore **mailboxMutexes;
Semaphore **freeSpacesSem;
int *unreadMessages;
//for task 4
int R;
int W;
int N;
int numRead = 0;
Semaphore *area = new Semaphore("Shared Area Semaphore", 1);
Semaphore *mutex = new Semaphore("General Mutual Exclusion Semaphore", 1);
//we want threads to execute in a particular order
Semaphore *readBlock = new Semaphore("Block Reading", 0);
Semaphore *writeBlock = new Semaphore("Block writing", 0);
Semaphore *waitReadersBlocked = new Semaphore("Wait For Last Reader", 0);
bool entFirst = 1;
int enteredRead = 0;
int finishedRead = 0;
int finishedWrite = 0;
int enteredWaiters = 0;
int firstIter = 1;
int toPass;
bool nextNotLast = 1;
//bonus global

class monitorDP
{
private:
    enum
    {
        THINKING,
        HUNGRY,
        EATING
    } state[10000];
    Condition **self;

    void test(int i)
    {
        if ((state[(i + P - 1) % P] != EATING) && (state[i] == HUNGRY) && (state[(i + 1) % P] != EATING))
        {
            printf("\n---Philosopher %d has picked up both chopsticks", i);
            state[i] = EATING;
            self[i]->Signal(self[i]->conditionLock);
            printf("\n----Philosopher %d is eating", i);
        }
    }

public:
    monitorDP();

    void pickUp(int i)
    {
        printf("\n-Philosopher %d is hungry", i);
        state[i] = HUNGRY;
        printf("\n--Philosopher %d is trying to pick up both chopsticks", i);
        test(i);
        if (state[i] != EATING)
        {
            self[i]->Wait(self[i]->conditionLock);
        }
    }

    void putDown(int i)
    {
        state[i] = THINKING;
        printf("\n------Philosopher %d is putting down left chopstick", i);
        test((i + P - 1) % P);
        printf("\n-------Philosopher %d is putting down right chopstick", i);
        test((i + 1) % P);
        printf("\n--------Philosopher %d is thinking", i);
    }
};

monitorDP::monitorDP()
{
    self = new Condition *[10000];
    for (int i = 0; i < 10000; i++)
    {
        self[i] = new Condition("ConditionVar");
        state[i] = THINKING;
    }
}

monitorDP *MDP = new monitorDP();

Semaphore *startMonitor = new Semaphore("Wait For Monitor Termination", 0);
Semaphore *endMonitor = new Semaphore("Wait For Monitor Termination", 0);
Semaphore *numThreads = new Semaphore("Protect N", 1);
//reuse N for numThreads
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
        chopSticksSem[which]->P();
        printf("\n-Philosopher %d has picked up left chopstick", which);
        chopSticksSem[(which + 1) % P]->P();
        printf("\n--Philosopher %d has picked up right chopstick", which);

        //hoard M for reading and writing
        mealSem->P();

        //there are no longer any meals, we can leave the loop
        if (!M)
        {
            mealSem->V();
            break;
        }

        //there are still meals left
        M--;
        printf("\n---Philosopher %d has begun eating. Meals remaining: %d", which, M);
        //release M once finished
        mealSem->V();
        //eat
        busyWait(3, 6);

        //drop chopsticks
        chopSticksSem[which]->V();
        printf("\n----Philosopher %d has dropped left chopstick", which);
        chopSticksSem[(which + 1) % P]->V();
        printf("\n-----Philosopher %d has dropped right chopstick", which);

        printf("\n------Philosopher %d has begun thinking", which);
        busyWait(3, 6);
    }
    //drop chopsticks (since the last iteration doesn't enter the first if condition)
    chopSticksSem[which]->V();
    printf("\n----Philosopher %d has dropped left chopstick", which);
    chopSticksSem[(which + 1) % P]->V();
    printf("\n-----Philosopher %d has dropped right chopstick", which);
    printf("\n-------Philosopher %d has finished their last meal", which);

    //NF starts from P (num of threads) and is decreased once per thread
    NFSem->P();
    notFinished--;
    //this following statement will only be true for the last thread
    if (notFinished == 0)
    {
        printf("\n\n-----------------All philosophers will now leave the table------------------\n\n");
        allLeaveSem->V();
    }
    NFSem->V();
    allLeaveSem->P();
    allLeaveSem->V();
    printf("--------Philosopher %d is exiting\n", which);
}

//task 3 postOffice code
char *getRandMsg()
{
    int chooseShout = Random() % 11;

    if (chooseShout == 0)
    {
        return "Sit down at table.";
    }
    else if (chooseShout == 1)
    {
        return "Pick up left chopstick.";
    }
    else if (chooseShout == 2)
    {
        return "Pick up right chopstick.";
    }
    else if (chooseShout == 3)
    {
        return "Begin eating.";
    }
    else if (chooseShout == 4)
    {
        return "Continue eating for 3-6 cycles.";
    }
    else if (chooseShout == 5)
    {
        return "Put down left chopstick.";
    }
    else if (chooseShout == 6)
    {
        return "Put down right chopstick.";
    }
    else if (chooseShout == 7)
    {
        return "Begin thinking.";
    }
    else if (chooseShout == 8)
    {
        return "Continue thinking for 3-6 cycles.";
    }
    else if (chooseShout == 9)
    {
        return "IF all meals have not been eaten, GOTO 2.";
    }
    else if (chooseShout == 10)
    {
        return "ELSE leave the table.";
    }
}

void postOffice(int which)
{
    while (true)
    {
        //beginning of the loop, start of algorithm
        printf("\n-Person %d entered the post office", which);

        //we are still running the algorithm, read messages in the mailbox until none are left
        while (true)
        {
            //check if there are any unread messages
            int messageIndex;
            mailboxMutexes[which]->P();
            //if no unread messages, unblock the semaphore leave the loop
            if (!unreadMessages[which])
            {
                mailboxMutexes[which]->V();
                break;
            }

            //decrement num of unread messages
            //store CURRENT value of unread messages to use as an index
            messageIndex = --unreadMessages[which];

            //at this point, we read out the message
            char *contents = mailboxes[which][messageIndex]->contents;
            int sender = mailboxes[which][messageIndex]->sender;
            mailboxes[which][messageIndex] = NULL;
            mailboxMutexes[which]->V();

            printf("\n--Person %d has read the message '%s' from Person %d", which, contents, sender);

            //signify that there is a free message space
            printf("\n---Person %d has called V() on their mailbox", which);
            freeSpacesSem[which]->V();

            //Yield as per the algorithm
            printf("\n----Person %d is yielding their turn", which);
            currentThread->Yield();

            printf("\n-----Person %d is checking for more messages to read", which);
        }

        printf("\n------Person %d's mailbox is empty", which);

        //we have finished with reading messages, now to compose one
        Message *toSend = new Message;

        char *contents = getRandMsg();

        // randomly decide which Person to send to, and compose a Message struct
        int reciever = -1;
        while (reciever == which || reciever == -1)
        {
            reciever = Random() % P;
        }
        toSend->contents = contents;
        toSend->sender = which;

        printf("\n------Person %d has composed the message '%s' for Person %d", which, contents, reciever);

        //block M before reading, and if M=0, exit the simulation
        MessageSemaphore->P();
        if (!M)
        {
            MessageSemaphore->V();
            printf("\n---------Person %d exited the post office because no more messages can be sent", which);
            break;
        }

        //decrement total M. if we are at this point, we are definitely sending the constructed message.
        M--;
        printf("\n==================================Total messages remaining: %d", M);

        MessageSemaphore->V();

        //signal that the reciever is losing a free slot
        freeSpacesSem[reciever]->P();
        //increase number of unread messages so the reciever knows where to read from first
        mailboxMutexes[reciever]->P();
        int recieverUnreadMessages = unreadMessages[reciever]++;
        //actually place the message into the mailbox of the reciever
        mailboxes[reciever][recieverUnreadMessages] = toSend;
        mailboxMutexes[reciever]->V();

        printf("\n-------Person %d sent the message '%s' to Person %d's mailbox and called P()", which, contents, reciever);
        printf("\n--------Person %d exited the post office", which);
        busyWait(3, 6);
    }
}

//task 4 reader/writer code
// void readerThread(int which)
// {
//     //wait for main to signal
//     readBlock->P();
//     //instantly increment number of entered readers
//     enteredRead++;

//     //if entered readers is one after the max number of readers per section
//     //AND if there are still writers
//     //block any other readers from entering

//     if (enteredRead == (N + 1) && (W - finishedWrite) != 0)
//     {
//         area->V();
//         waitReadersBlocked->V();
//         readBlock->P();
//     }

//     //take area semaphore if first reader
//     if (enteredRead == 1)
//     {
//         area->P();
//     }
//     readBlock->V();

//     printf("\n--READ #%d BEGINS", which);
//     // for (int i = 0; i < 5000000; i++)
//     //     ;
//     printf("\n--READ #%d FINISH", which);

//     mutex->P();
//     //this thread is finished reading(protected)
//     finishedRead++;

//     //if no more writers, switch to readers
//     if ((W - finishedWrite) == 0)
//     {
//         readBlock->V();
//     }
//     //if entered readers is equal to max number of readers per section
//     //AND if there are still writers
//     //OR there are no more writers
//     //release area and allow writers to take control
//     else if (((R - finishedRead) == 0))
//     {
//         area->V();
//         writeBlock->V();
//     }
//     else if ((finishedRead % N == 0 && (W - finishedWrite) != 0))
//     {
//         waitReadersBlocked->P();
//         writeBlock->V();
//     }

//     mutex->V();
// }

void readerThread(int which)
{
    //wait for main to signal
    readBlock->P();

    mutex->P();
    enteredRead++;
    if (enteredRead == (toPass + 1) && W - finishedWrite != 0)
    {
        if (firstIter)
        {
            waitReadersBlocked->V();
        }
        enteredWaiters++;
        mutex->V();
        readBlock->P();
    }
    else
    {
        mutex->V();
    }
    //instantly increment number of entered readers
    //if we're at max readers, and there are writers, block next readers

    //printf("\n%dWAITINGON4", which);
    mutex->P();
    if (entFirst)
    {
        entFirst = 0;
        mutex->V();
        area->P();
    }
    else
    {
        mutex->V();
    }

    printf("\n--READ #%d BEGINS", which);
    // for (int i = 0; i < 10000000; i++)
    //     ;
    printf("\n--READ #%d FINISH", which);

    //printf("\n%dWAITINGON6", which);
    mutex->P();
    finishedRead++;
    if ((finishedRead % (N) == 0 && (W - finishedWrite) != 0))
    {

        if (firstIter && R - finishedRead != 0)
        {
            mutex->V();
            readBlock->V();
            waitReadersBlocked->P();
            mutex->P();
            firstIter = 0;
            mutex->V();
        }
        else
        {
            mutex->V();
        }
        writeBlock->V();
        area->V();
    }
    else if (R - finishedRead == 0)
    {
        mutex->V();
        writeBlock->V();
    }
    else
    {
        mutex->V();
        readBlock->V();
    }
}

void writerThread(int which)
{
    //wait to be signaled by reader or by writer thread if no readers
    writeBlock->P();
    area->P();

    printf("\n-----WRITE #%d BEGINS", which);
    // for (int i = 0; i < 300000000; i++)
    //     ;
    printf("\n-----WRITE #%d FINISH", which);

    //writing is finished, increment
    finishedWrite++;

    if (R - finishedRead != 0 && W - finishedWrite != 0)
    {
        mutex->P();
        entFirst = 1;
        enteredRead = 0;
        toPass = N - enteredWaiters + 1;
        for (int i = 0; i < enteredWaiters; i++)
        {
            readBlock->V();
        }
        mutex->V();
    }

    //if no more readers, sequentially unblock writers until death
    if ((R - finishedRead) == 0)
    {
        writeBlock->V();
    }
    //release area no matter what
    area->V();

    //if readers exist, allow them to run
    if ((R - finishedRead) != 0 && (W - finishedWrite) == 0)
    {
        for (int i = 0; i < R; i++)
        {
            readBlock->V();
        }
    }
}

//bonus task
void monitorDPThread(int which)
{
    while (true)
    {
        startMonitor->P();
        startMonitor->V();
        mealSem->P();
        if (!M)
        {
            mealSem->V();
            break;
        }
        M--;
        int localM = M;
        mealSem->V();

        MDP->pickUp(which);
        busyWait(3, 6);
        printf("\n-----after %d ate, M is now %d", which, localM);
        MDP->putDown(which);
        busyWait(3, 6);
    }
    numThreads->P();
    N--;
    numThreads->V();
    endMonitor->P();
    printf("\nPhilosopher %d is leaving", which);
    endMonitor->V();
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
                chopSticksSem = new Semaphore *[P];
                int threadNameMaxLen = strlen("Chopstick Semaphore 99999");
                char *buf = new char[threadNameMaxLen];
                for (int i = 0; i < P; i++)
                {
                    snprintf(buf, threadNameMaxLen, "Chopstick Semaphore %d", i);
                    chopSticksSem[i] = new Semaphore(buf, 1);
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
        //prompt and capture valid input
        //arrSize input of 7 should be plenty (99999) to support 10k
        int arrSize = 7;
        char *MInput = new char[arrSize];
        char *PInput = new char[arrSize];
        char *SInput = new char[arrSize];

        //M validation
        bool validInput = false;
        printf("enter max number of messages to be sent, whitespaces don't count and input bytesize must be %d or less: ", arrSize);
        fgets(MInput, arrSize, stdin);

        while (!validInput)
        {
            //input size > max size is invalid
            if (inputOverflow(MInput))
            {
                printf("input overflow error. enter max number of messages to be sent (size %d): ", arrSize);
                fgets(MInput, arrSize, stdin);
            }
            //only integers are valid
            else if (!isInteger(MInput))
            {
                printf("that was not an integer. enter max number of messages to be sent (size %d): ", arrSize);
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
        printf("enter number of people to enter post office, whitespaces don't count and input bytesize must be %d or less: ", arrSize);
        fgets(PInput, arrSize, stdin);

        while (!validInput)
        {
            //input size > max size is invalid
            if (inputOverflow(PInput))
            {
                printf("input overflow error. enter number of people to enter post office (size %d): ", arrSize);
                fgets(PInput, arrSize, stdin);
            }
            //only integers are valid
            else if (!isInteger(PInput))
            {
                printf("that was not an integer. enter number of people to enter post office (size %d): ", arrSize);
                fgets(PInput, arrSize, stdin);
            }
            //if P = 1, P0 cannot send message to a thread that isn't itself
            else if (atoi(PInput) == 1)
            {
                printf("P cannot be equal to 1. enter number of people to enter post office (size %d): ", arrSize);
                fgets(PInput, arrSize, stdin);
            }
            //input is valid, accept input
            else
            {
                validInput = true;
                P = atoi(PInput);
            }
        }

        //S validation
        validInput = false;
        printf("enter number of messages per mailbox, whitespaces don't count and input bytesize must be %d or less: ", arrSize);
        fgets(SInput, arrSize, stdin);

        while (!validInput)
        {
            //input size > max size is invalid
            if (inputOverflow(SInput))
            {
                printf("input overflow error. enter number of messages per mailbox (size %d): ", arrSize);
                fgets(SInput, arrSize, stdin);
            }
            //only integers are valid
            else if (!isInteger(SInput))
            {
                printf("that was not an integer. enter number of messages per mailbox (size %d): ", arrSize);
                fgets(SInput, arrSize, stdin);
            }
            //if P = 1, P0 cannot send message to a thread that isn't itself
            else if (atoi(SInput) == 0)
            {
                printf("S cannot be equal to 0. enter number of people to enter post office (size %d): ", arrSize);
                fgets(SInput, arrSize, stdin);
            }
            //input is valid, accept input
            else
            {
                validInput = true;
                S = atoi(SInput);
            }
        }

        //input has been validated
        //after collecting P and M, construct the mailbox filled with null
        mailboxes = new Message **[P];
        for (int i = 0; i < P; i++)
        {
            mailboxes[i] = new Message *[S];
            for (int j = 0; j < S; j++)
            {
                mailboxes[i][j] = NULL;
            }
        }

        //initialize mailboxMutexes, to control access to a particular mailbox
        mailboxMutexes = new Semaphore *[P];
        for (int i = 0; i < P; i++)
        {
            int threadNameMaxLen = strlen("Mailbox Mutex 99999");
            char *buf = new char[threadNameMaxLen];
            snprintf(buf, threadNameMaxLen, "Mailbox Mutex %d", i);
            mailboxMutexes[i] = new Semaphore(buf, 1);
        }

        //initialize freeSpacesSem, to prevent threads from writing to full mailboxes
        freeSpacesSem = new Semaphore *[P];
        for (int i = 0; i < P; i++)
        {
            int threadNameMaxLen = strlen("Free Space Semaphore 99999");
            char *buf = new char[threadNameMaxLen];
            snprintf(buf, threadNameMaxLen, "Free Space For Mailbox %d", i);
            freeSpacesSem[i] = new Semaphore(buf, S);
        }

        //initialize unreadMessages, acts as a pointer to show where a mailbox can be read and written to
        if (P)
        {
            unreadMessages = new int[P];
            for (int i = 0; i < P; i++)
            {
                unreadMessages[i] = 0;
            }
        }

        //fork all people threads
        for (int i = 0; i < P; i++)
        {
            int threadNameMaxLen = strlen("Person Thread 99999");
            char *buf = new char[threadNameMaxLen];
            snprintf(buf, threadNameMaxLen, "Person Thread %d", i);

            Thread *t = new Thread(buf);
            t->Fork(postOffice, i);
        }
    }

    else if (projTask == 6)
    {
        //prompt and capture valid input
        //arrSize input of 7 should be plenty (99999) to support 10k
        int arrSize = 7;
        char *RInput = new char[arrSize];
        char *WInput = new char[arrSize];
        char *NInput = new char[arrSize];

        //R validation
        bool validInput = false;
        printf("note: simulated latency can be removed by deleting for loop between read/write printf\n\n");
        printf("enter max number of readers in general, whitespaces don't count and input bytesize must be %d or less: ", arrSize);
        fgets(RInput, arrSize, stdin);

        while (!validInput)
        {
            //input size > max size is invalid
            if (inputOverflow(RInput))
            {
                printf("input overflow error. enter max number of readers in general (size %d): ", arrSize);
                fgets(RInput, arrSize, stdin);
            }
            //only integers are valid
            else if (!isInteger(RInput))
            {
                printf("that was not an integer. enter max number of readers in general (size %d): ", arrSize);
                fgets(RInput, arrSize, stdin);
            }
            //input is valid, accept input
            else
            {
                validInput = true;
                R = atoi(RInput);
            }
        }

        //W validation
        validInput = false;
        printf("enter max number of writers, whitespaces don't count and input bytesize must be %d or less: ", arrSize);
        fgets(WInput, arrSize, stdin);

        while (!validInput)
        {
            //input size > max size is invalid
            if (inputOverflow(WInput))
            {
                printf("input overflow error. enter max number of writers (size %d): ", arrSize);
                fgets(WInput, arrSize, stdin);
            }
            //only integers are valid
            else if (!isInteger(WInput))
            {
                printf("that was not an integer. enter max number of writers (size %d): ", arrSize);
                fgets(WInput, arrSize, stdin);
            }
            //input is valid, accept input
            else
            {
                validInput = true;
                W = atoi(WInput);
            }
        }

        //N validation
        validInput = false;
        printf("enter max number of readers accessing file, whitespaces don't count and input bytesize must be %d or less: ", arrSize);
        fgets(NInput, arrSize, stdin);

        while (!validInput)
        {
            //input size > max size is invalid
            if (inputOverflow(NInput))
            {
                printf("input overflow error. enter max number of readers accessing file (size %d): ", arrSize);
                fgets(NInput, arrSize, stdin);
            }
            //only integers are valid
            else if (!isInteger(NInput))
            {
                printf("that was not an integer. enter max number of readers accessing file (size %d): ", arrSize);
                fgets(NInput, arrSize, stdin);
            }
            //input is valid, accept input
            else
            {
                validInput = true;
                N = atoi(NInput);
                toPass = N;
            }
        }

        //validation complete
        //forking threads
        for (int i = 0; i < R; i++)
        {
            int threadNameMaxLen = strlen("Reader Thread 99999");
            char *buf = new char[threadNameMaxLen];
            snprintf(buf, threadNameMaxLen, "Reader Thread %d", i);

            Thread *t = new Thread(buf);
            t->Fork(readerThread, i);
        }

        for (int i = 0; i < W; i++)
        {
            int threadNameMaxLen = strlen("Writer Thread 99999");
            char *buf = new char[threadNameMaxLen];
            snprintf(buf, threadNameMaxLen, "Writer Thread %d", i);

            Thread *t = new Thread(buf);
            t->Fork(writerThread, i);
        }

        //allow readers to read
        readBlock->V();
        //allow writers if there are no readers
        if (!R)
            writeBlock->V();
    }

    else if (projTask == 0)
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
                N = P;
            }
        }

        //input validation complete
        for (int i = 0; i < P; i++)
        {
            int threadNameMaxLen = strlen("Philosopher Thread 1000000");
            char *buf = new char[threadNameMaxLen];
            snprintf(buf, threadNameMaxLen, "Philosopher Thread %d", i);

            Thread *t = new Thread(buf);
            t->Fork(monitorDPThread, i);
            printf("\nPhilosopher %d is waiting to be seated", i);
        }
        startMonitor->V();
        printf("\n\n==================All philosophers are seated========================\n");
        while (true)
        {
            numThreads->P();
            if (!N)
            {
                break;
            }
            numThreads->V();
        }
        printf("\n\n==================All philosophers are seated========================\n");
        endMonitor->V();
    }

    //End proj2 code changes by Lucas Blanchard

    //yield and end main thread once forked, or once illegal projTask value is passed
    currentThread->Finish();
}
//End proj1 code changes by Lucas Blanchard