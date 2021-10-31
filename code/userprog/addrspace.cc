// addrspace.cc
//	Routines to manage address spaces (executing user programs).
//
//	In order to run a user program, you must:
//
//	1. link with the -N -T 0 option
//	2. run coff2noff to convert the object file to Nachos format
//		(Nachos object code format is essentially just a simpler
//		version of the UNIX executable object code format)
//	3. load the NOFF file into the Nachos file system
//		(if you haven't implemented the file system yet, you
//		don't need to do this last step)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "addrspace.h"
#include "bitmap.h"
#include "list.h"
#ifdef HOST_SPARC
#include <strings.h>
#endif

//----------------------------------------------------------------------
// SwapHeader
// 	Do little endian to big endian conversion on the bytes in the
//	object file header, in case the file was generated on a little
//	endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------
// taslk

//begin code changes by Michael Rivera
static BitMap *memoryMap = new BitMap(NumPhysPages);
static List *FIFOList = new List();
static Thread **IPT = new Thread *[NumPhysPages];
//int currentBit = 0;
//end code changes by Michael Rivera
static void
SwapHeader(NoffHeader *noffH)
{
    noffH->noffMagic = WordToHost(noffH->noffMagic);
    noffH->code.size = WordToHost(noffH->code.size);
    noffH->code.virtualAddr = WordToHost(noffH->code.virtualAddr);
    noffH->code.inFileAddr = WordToHost(noffH->code.inFileAddr);
    noffH->initData.size = WordToHost(noffH->initData.size);
    noffH->initData.virtualAddr = WordToHost(noffH->initData.virtualAddr);
    noffH->initData.inFileAddr = WordToHost(noffH->initData.inFileAddr);
    noffH->uninitData.size = WordToHost(noffH->uninitData.size);
    noffH->uninitData.virtualAddr = WordToHost(noffH->uninitData.virtualAddr);
    noffH->uninitData.inFileAddr = WordToHost(noffH->uninitData.inFileAddr);
}

//----------------------------------------------------------------------
// AddrSpace::AddrSpace
// 	Create an address space to run a user program.
//	Load the program from a file "executable", and set everything
//	up so that we can start executing user instructions.
//
//	Assumes that the object code file is in NOFF format.
//
//	First, set up the translation from program memory to physical
//	memory.  For now, this is really simple (1:1), since we are
//	only uniprogramming, and we have a single unsegmented page table
//
//	"executable" is the file containing the object code to load into memory
//----------------------------------------------------------------------
//begin code changes by Michael Rivera
//begin code changes by Michael Rivera
AddrSpace::AddrSpace(OpenFile *executable, int PID)
{
    unsigned int i, size;
    //assign member PID
    this->PID = PID;

    executable->ReadAt((char *)&noffH, sizeof(noffH), 0);
    if ((noffH.noffMagic != NOFFMAGIC) &&
        (WordToHost(noffH.noffMagic) == NOFFMAGIC))
        SwapHeader(&noffH);
    if (noffH.noffMagic == NOFFMAGIC)
    {

        // how big is address space?
        size = noffH.code.size + noffH.initData.size + noffH.uninitData.size + UserStackSize; // we need to increase the size
                                                                                              // to leave room for the stack
        numPages = divRoundUp(size, PageSize);

        //Begin group code changes
        int threadNameMaxLen = strlen("100000000000000.swap");
        swapName = new char[threadNameMaxLen];
        snprintf(swapName, threadNameMaxLen, "%d.swap", PID);

        //create and open swap
        fileSystem->Create(swapName, size);
        swap = fileSystem->Open(swapName);

        //use buffer to bring essential information from exec to swap
        int bufSize = noffH.code.size + noffH.initData.size + noffH.uninitData.size;
        char *tempSwapCodeBuf = new char[bufSize];
        executable->ReadAt(tempSwapCodeBuf, bufSize, sizeof(noffH));
        swap->WriteAt(tempSwapCodeBuf, bufSize, 0);

        delete tempSwapCodeBuf;
        //End group code changes

        size = numPages * PageSize;

        DEBUG('a', "Initializing address space, num pages %d, size %d\n",
              numPages, size);
        // first, set up the translation
        pageTable = new TranslationEntry[numPages];

        for (i = 0; i < numPages; i++)
        {
            pageTable[i].virtualPage = i;
            pageTable[i].physicalPage = -1;
            pageTable[i].valid = FALSE;
            pageTable[i].use = FALSE;
            pageTable[i].dirty = FALSE;
            pageTable[i].readOnly = FALSE; // if the code segment was entirely on
                                           // a separate page, we could set its
                                           // pages to be read-only
        }
    }
    else
    {
        printf("\nFile was not recognized as a Nachos object code file\n");
    }
}

// //tests the bits that the process has taken and clears them, pass a parameter of the each bit that a certain process has taken
// void AddrSpace::clearBitsForAProcess(int bitToClear)
// {
//     if (memoryMap->Test(bitToClear) == TRUE)
//     {
//         memoryMap->Clear(bitToClear);
//     }
// }
//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
// 	Dealloate an address space.  Nothing for now!
//----------------------------------------------------------------------

AddrSpace::~AddrSpace()
{
    //Begin code changes by Lucas Blanchard
    //clear page bits used by exiting process
    if (outputUserProg)
    {
        printf("Process %d before deallocation\n", PID);
        memoryMap->Print();
    }
    for (int i = 0; i < numPages; i++)
    {
        if (pageTable[i].valid)
        {
            int physPageNum = pageTable[i].physicalPage;
            memoryMap->Clear(physPageNum);
        }
    }
    delete pageTable;
    //actually delete the actual swap file on exit
    fileSystem->Remove(swapName);
    delete swap;
    delete swapName;
    if (outputUserProg)
    {
        printf("\nProcess %d after deallocation\n", PID);
        memoryMap->Print();
        printf("\n\n");
    }
    //End code changes by Lucas Blanchard
}

void AddrSpace::PrintSwap()
{
    int printPageSize = noffH.code.size + noffH.initData.size + noffH.uninitData.size + UserStackSize;
    numPages = divRoundUp(printPageSize, PageSize);

    printf("\nCODE SIZE: %d\nINIT SIZE: %d\nUNINIT SIZE: %d\nSTACK SIZE: %d\n", noffH.code.size, noffH.initData.size, noffH.uninitData.size, UserStackSize);
    printf("\nCODE PAGES: %d\nINIT PAGES: %d\nUNINIT PAGES: %d\nSTACK PAGES: %d\n", divRoundUp(noffH.code.size, PageSize), divRoundUp(noffH.initData.size, PageSize), divRoundUp(noffH.uninitData.size, PageSize), divRoundUp(UserStackSize, PageSize));

    char *tempSwapCodeBuf = new char[printPageSize];
    swap->ReadAt(tempSwapCodeBuf, printPageSize, sizeof(noffH));
    //swap->WriteAt(tempSwapCodeBuf, bufSize, 0);
    printf("\n\nSwap:\n");
    for (int pageNum = 0; pageNum < numPages; pageNum++)
    {
        printf("Page Num %d: ", pageNum);
        for (int i = 0; i < PageSize; i++)
        {
            printf("%d ", tempSwapCodeBuf[(pageNum * PageSize) + i]);
        }
        printf("\n\n");
    }
    printf("\n");
    delete tempSwapCodeBuf;
}

void AddrSpace::PrintTable()
{
    for (int i = 0; i < numPages; i++)
    {
        printf("\nVP: %d||PP: %d||Valid: %d", i, pageTable[i].physicalPage, pageTable[i].valid);
    }
}

//Begin code changes by Lucas Blanchard
void AddrSpace::loadPage(int vPageNum, unsigned int virtualAddr)
{
    if (outputUserProg)
    {
        printf("\nPage fault: Process %d is requesting virtual page %d", PID, vPageNum);
    }

    //test and mark free bit
    int physPageNum = memoryMap->Find();
    int offset = virtualAddr % PageSize;
    //if free page, enter
    if (physPageNum != -1)
    {
        if (outputUserProg)
        {
            printf("\nAssigning physical page %d\n", physPageNum);
            memoryMap->Print();
            printf("\n\n");
        }
        //add to FIFO list
        FIFOList->Append((void *)physPageNum);
        //update IPT
        IPT[physPageNum] = currentThread;

        //update PT
        pageTable[vPageNum].physicalPage = physPageNum;
        pageTable[vPageNum].valid = TRUE;

        //load from swap
        swap->ReadAt(&(machine->mainMemory[physPageNum * PageSize]), PageSize, vPageNum * PageSize);
    }
    //no free page, check replacement type
    else
    {
        //begin group code changes
        if (replacementType == 1)
        {
            //grab phys from list
            physPageNum = (int)(FIFOList->Remove());
            //grab thread holding phys from IPT
            Thread *threadToKick = IPT[physPageNum];

            if (outputUserProg)
            {
                printf("\nSwapping physical page %d from process %d", physPageNum, threadToKick->space->PID);
            }

            //tell that thread to save to swap its VP holding its frame
            threadToKick->space->saveToSwap(physPageNum);
            memoryMap->Mark(physPageNum);          //claim bitmap
            FIFOList->Append((void *)physPageNum); //add new page to FIFO list

            //load new page, update IPT And PT
            swap->ReadAt(&(machine->mainMemory[physPageNum * PageSize]), PageSize, vPageNum * PageSize);
            IPT[physPageNum] = currentThread;
            pageTable[vPageNum].physicalPage = physPageNum;
            pageTable[vPageNum].valid = TRUE;

            if (outputUserProg)
            {
                printf("Virtual page %d added\n", vPageNum);
                memoryMap->Print();
                printf("\n\n");
            }
        }
        else if (replacementType == 2)
        {
            //random num 0 to numphyspages - 1
            physPageNum = Random() % (NumPhysPages);
            //grab thread from IPT
            Thread *threadToKick = IPT[physPageNum];

            if (outputUserProg)
            {
                printf("\nSwapping physical page %d from process %d", physPageNum, threadToKick->space->PID);
            }

            //tell other thread to save its VP and laim map
            threadToKick->space->saveToSwap(physPageNum);
            memoryMap->Mark(physPageNum);

            //load new page, update IPT and PT

            swap->ReadAt(&(machine->mainMemory[physPageNum * PageSize]), PageSize, vPageNum * PageSize);
            IPT[physPageNum] = currentThread;

            pageTable[vPageNum].physicalPage = physPageNum;
            pageTable[vPageNum].valid = TRUE;

            if (outputUserProg)
            {
                printf("Virtual page %d added\n", vPageNum);
                memoryMap->Print();
                printf("\n\n");
            }
            //end group code changes
        }
        //no replacement, terminate process
        else
        {
            printf("\nERROR: not enough free frames. Terminating process %d", currentThread->getID());
            //release bits before terminating thread
            for (int i = 0; i < numPages; i++)
            {
                if (pageTable[i].valid)
                {
                    memoryMap->Clear(pageTable[i].physicalPage);
                    if (outputUserProg)
                    {
                        printf("\nVirtual Page %d removed", i);
                    }
                }
            }
            printf("\n\n");
            currentThread->Finish();
        }
    }
}

void AddrSpace::saveToSwap(int physPageNum)
{
    //search process page table for VALID VP holding a frame in mem
    for (int i = 0; i < numPages; i++)
    {
        if (pageTable[i].physicalPage == physPageNum && pageTable[i].valid == TRUE)
        {
            //update PT and save to swap
            pageTable[i].valid = FALSE;
            swap->WriteAt(&(machine->mainMemory[physPageNum * PageSize]), PageSize, i * PageSize);
            memoryMap->Clear(physPageNum); //let room up for bitmap in replacement frame
            if (outputUserProg)
            {
                printf("\nVirtual page %d removed\n", i);
                memoryMap->Print();
                printf("\n");
            }
        }
    }
}

//End code changes by Lucas Blanchard

//----------------------------------------------------------------------
// AddrSpace::InitRegisters
// 	Set the initial values for the user-level register set.
//
// 	We write these directly into the "machine" registers, so
//	that we can immediately jump to user code.  Note that these
//	will be saved/restored into the currentThread->userRegisters
//	when this thread is context switched out.
//----------------------------------------------------------------------

void AddrSpace::InitRegisters()
{
    int i;

    for (i = 0; i < NumTotalRegs; i++)
        machine->WriteRegister(i, 0);

    // Initial program counter -- must be location of "Start"
    machine->WriteRegister(PCReg, 0);

    // Need to also tell MIPS where next instruction is, because
    // of branch delay possibility
    machine->WriteRegister(NextPCReg, 4);

    // Set the stack register to the end of the address space, where we
    // allocated the stack; but subtract off a bit, to make sure we don't
    // accidentally reference off the end!
    machine->WriteRegister(StackReg, numPages * PageSize - 16);
    DEBUG('a', "Initializing stack register to %d\n", numPages * PageSize - 16);
}

//----------------------------------------------------------------------
// AddrSpace::SaveState
// 	On a context switch, save any machine state, specific
//	to this address space, that needs saving.
//
//	For now, nothing!
//----------------------------------------------------------------------

void AddrSpace::SaveState()
{
}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
// 	On a context switch, restore the machine state so that
//	this address space can run.
//
//      For now, tell the machine where to find the page table.
//----------------------------------------------------------------------

void AddrSpace::RestoreState()
{
    machine->pageTable = pageTable;
    machine->pageTableSize = numPages;
}
