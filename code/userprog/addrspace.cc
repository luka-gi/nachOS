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

        fileSystem->Create(swapName, size);
        swap = fileSystem->Open(swapName);

        int bufSize = noffH.code.size + noffH.initData.size + noffH.uninitData.size;
        char *tempSwapCodeBuf = new char[bufSize];
        executable->ReadAt(tempSwapCodeBuf, bufSize, sizeof(noffH));
        swap->WriteAt(tempSwapCodeBuf, bufSize, 0);

        delete tempSwapCodeBuf;
        //End group code changes

        size = numPages * PageSize;
        // check we're not trying
        // to run anything too big --
        // at least until we have
        // virtual memory
        if (numPages > NumPhysPages)
        {
            printf("Error not enough virtual memory to run");
            //do something maybe different idk, make it only terminate one process somehow
            Exit(-1);
        }

        DEBUG('a', "Initializing address space, num pages %d, size %d\n",
              numPages, size);
        // first, set up the translation
        pageTable = new TranslationEntry[numPages];

        for (i = 0; i < numPages; i++)
        {
            pageTable[i].virtualPage = i; // for now, virtual page # = phys page #
            pageTable[i].physicalPage = -1;
            pageTable[i].valid = FALSE;
            pageTable[i].use = FALSE;
            pageTable[i].dirty = FALSE;
            pageTable[i].readOnly = FALSE; // if the code segment was entirely on
                                           // a separate page, we could set its
                                           // pages to be read-only
        }

        memoryMap->Print();

        //show current memory map bits that are taken up
        //}

        //CHANGE THE ZERO FUNCTION TO ONLY 0 OUT THE PAGES THAT ARE TO BE USED PER PROCESS(HANDLED IN PAGE FAULT EXCEPTIONS)

        // zero out the entire address space, to zero the unitialized data segment
        // and the stack segment

        //PRINT FOR DEBUGGING
        //printf("numVPages: %d\n", numPages);
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
    //clear pages used by exiting process
    if (outputUserProg)
    {
        printf("\nProcess %d Pre deallocation", PID);
        memoryMap->Print();
    }
    for (int i = 0; i < numPages; i++)
    {
        if (pageTable[i].valid)
        {
            memoryMap->Clear(pageTable[i].physicalPage);
        }
    }
    delete pageTable;
    //actually delete the actual file
    fileSystem->Remove(swapName);
    delete swap;
    delete swapName;
    if (outputUserProg)
    {
        printf("Process %d Post deallocation", PID);
        memoryMap->Print();
    }
    //End code changes by Lucas Blanchard
}

//Begin code changes by Lucas Blanchard
void AddrSpace::loadPage(int vPageNum, unsigned int virtualAddr)
{
    //then, copy in the code and data segments into memory

    if (outputUserProg)
    {
        printf("\nPage fault: Process %d is requesting virtual page %d\n", PID, vPageNum);
    }

    int physPageNum = memoryMap->Find();
    int offset = virtualAddr % PageSize;
    //Then, find the first available free page (no need for special memory allocation)
    //and update the currentThread’s pageTable[].physicalPage
    if (physPageNum != -1)
    {
        if (outputUserProg)
        {
            printf("\nAssigning physical page %d\n", physPageNum);
            memoryMap->Print();
        }
        FIFOList->Append((void *)physPageNum);
        IPT[physPageNum] = currentThread;

        pageTable[vPageNum].physicalPage = physPageNum;
        pageTable[vPageNum].valid = TRUE;

        //print statement for debugging
        //printf("\nppn: %d\nvpn: %d\npageSize: %d\nvaddr: %d\nnoffset: %d\ninstroff: %d\n", physPageNum, vPageNum, PageSize, virtualAddr, noffH.code.inFileAddr, offset);
        swap->ReadAt(&(machine->mainMemory[physPageNum * PageSize]), PageSize, vPageNum * PageSize);
        //machine->PrintMemory();
    }
    else
    {
        if (outputUserProg)
        {
            //Swap out physical page Y from process <ONUM>.
            //Virtual page Z removed.
        }
        if (replacementType == 1)
        {
            physPageNum = (int)(FIFOList->Remove());
            Thread *threadToKick = IPT[physPageNum];
            threadToKick->space->saveToSwap(physPageNum);
            memoryMap->Mark(physPageNum);

            FIFOList->Append((void *)physPageNum);
            IPT[physPageNum] = currentThread;

            pageTable[vPageNum].physicalPage = physPageNum;
            pageTable[vPageNum].valid = TRUE;

            //print statement for debugging
            //printf("\nppn: %d\nvpn: %d\npageSize: %d\nvaddr: %d\nnoffset: %d\ninstroff: %d\n", physPageNum, vPageNum, PageSize, virtualAddr, noffH.code.inFileAddr, offset);

            swap->ReadAt(&(machine->mainMemory[physPageNum * PageSize]), PageSize, vPageNum * PageSize);

            if (outputUserProg)
            {
                printf("\nSwapping physical page %d from process %d\n", physPageNum, threadToKick->space->PID);
                memoryMap->Print();
            }
        }
        else if (replacementType == 2)
        {
            physPageNum = Random() % (NumPhysPages);

            Thread *threadToKick = IPT[physPageNum];
            threadToKick->space->saveToSwap(physPageNum);
            memoryMap->Mark(physPageNum);

            IPT[physPageNum] = currentThread;

            pageTable[vPageNum].physicalPage = physPageNum;
            pageTable[vPageNum].valid = TRUE;

            //print statement for debugging
            //printf("\nppn: %d\nvpn: %d\npageSize: %d\nvaddr: %d\nnoffset: %d\ninstroff: %d\n", physPageNum, vPageNum, PageSize, virtualAddr, noffH.code.inFileAddr, offset);

            swap->ReadAt(&(machine->mainMemory[physPageNum * PageSize]), PageSize, vPageNum * PageSize);

            if (outputUserProg)
            {
                printf("\nSwapping physical page %d from process %d\n", physPageNum, threadToKick->space->PID);
                memoryMap->Print();
            }
        }
        else
        {
            printf("\nCurrently there are not enough free frames. Terminating process %d\n", currentThread->getID());
            for (int i = 0; i < numPages; i++)
            {
                if (pageTable[i].valid)
                {
                    memoryMap->Clear(pageTable[i].physicalPage);
                    if (outputUserProg)
                    {
                        printf("\nVirtual Page %d Removed\n", i);
                    }
                }
            }
            currentThread->Finish();
        }
    }
}

void AddrSpace::saveToSwap(int physPageNum)
{
    for (int i = 0; i < numPages; i++)
    {
        if (pageTable[i].physicalPage == physPageNum)
        {
            //move phys into swap[virtual]
            swap->WriteAt(&(machine->mainMemory[physPageNum * PageSize]), PageSize, i * PageSize);
            pageTable[i].valid = FALSE;
            memoryMap->Clear(physPageNum);
            if (outputUserProg)
            {
                printf("\nMoved VP %d to swap from PP %d\n", i, physPageNum);
                memoryMap->Print();
            }
            break;
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
