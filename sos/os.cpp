#include <iostream>
#include <list>
#include <vector>
#include <map>
#include <queue>
#include "Job.h"

using namespace std;

// Data structures and global variables
std::vector<Job> JOBTABLE;
std::map<int, int> FREESPACETABLE; // address and size pair
std::queue<int*> readyq;
std::queue<int*> ioQueue;

void siodisk(int jobnum);
void initFST();
void addJobToJobtable (long jobNumber, long priority, long jobSize, long maxCpuTime, long currTime);
void addJobToFST(long jobNumber);
void printJobtable();
void printFST();
int  findFreeSpace(int jobSize);
void clearSpace(int index);
void clearSpace(int startIndex, int endIndex);
int  findJob(long jobNum);
void swapper(long jobNumber);

void siodrum(int jobnum, int jobsize, int coreaddress, int direction){
 // Channel commands siodisk and siodrum are made available to you by the simulator.
 // siodisk has one argument: job number, of type int and passed by value.
 // siodrum has four arguments, all of type int and passed by value:
 // first argument is job number;
 // second argument is job size;
 // third argument is starting core address;
 // fourth argument is interpreted as follows:
 // 1 => move from core (memory) to drum
 // 0 => move from drum to core (memory)


}

void ontrace(); // called without arguments
void offtrace(); // called without arguments
 // The 2 trace procedures allow you to turn the tracing mechanism on and off.
 // The default value is off. WARNING: ontrace produces a blow-by-blow description
 // of each event and results in an extremely large amount of output.
 // It should be used only as an aid in debugging.
 // Even with the trace off, performance statistics are
 // generated at regular intervals and a diagnostic message appears in case of a crash.
 // In either case, your OS need not print anything.

void startup()
{
 // Allows initialization of static system variables declared above.
 // Called once at start of the simulation.
    ontrace();
    initFST();

}

// INTERRUPT HANDLERS
    // The following 5 functions are the interrupt handlers. The arguments
    // passed from the environment are detailed with each function below.
    // See RUNNING A JOB, below, for additional information

void Crint (long &a, long p[])
{
    // Indicates the arrival of a new job on the drum.
    // At call: p [1] = job number
    // p [2] = priority
    // p [3] = job size, K bytes
    // p [4] = max CPU time allowed for job
    // p [5] = current time

    addJobToJobtable(p[1], p[2], p[3], p[4], p[5]);
    swapper(p[1]);

    printJobtable();
    printFST();
}

void Dskint (long &a, long p[])
{
 // Disk interrupt.
 // At call: p [5] = current time
}

void Drmint (long &a, long p[])
{
 // Drum interrupt.
 // At call: p [5] = current time

}

void Tro (long &a, long p[])
{
 // Timer-Run-Out.
 // At call: p [5] = current time
}
void Svc (long &a, long p[])
{
 // Supervisor call from user program.
 // At call: p [5] = current time
 // a = 5 => job has terminated
 // a = 6 => job requests disk i/o
 // a = 7 => job wants to be blocked un 5til all its pending
 // I/O requests are completed

}

// Create 100 elements in FST
void initFST()
{
    for (int i = 0; i < 100; i++)
    {
        FREESPACETABLE.insert(pair<int, int> (i, 0));
    }
}

void addJobToJobtable (long jobNumber, long priority, long jobSize, long maxCpuTime, long currTime)
{
    // Place the parameters in a temp job container
    Job newJob(jobNumber, jobSize, maxCpuTime, currTime, priority);

    // Add the temp vector to the JOBTABLE
    JOBTABLE.push_back(newJob);
}



void printJobtable()
{
    cout << "\n -- JOBTABLE --" << endl;
    for (int i = 0; i < JOBTABLE.size(); i++)
    {
        cout << "Job Num: " << JOBTABLE[i].getJobNumber() << '\t'
             << "Job Size: " << JOBTABLE[i].getJobSize() << '\t'
             << endl;
    }
}

void printFST()
{
    cout << "\n -- FREE SPACE TABLE -- " << endl;
    cout << "Addr   Size" << endl;
    map<int, int>::iterator fstIter;
    int i = 0;

    for (fstIter = FREESPACETABLE.begin(); fstIter != FREESPACETABLE.end(); fstIter++)
    {
        if (i < 10)
           cout << fstIter->first << "   =>  " << fstIter->second << endl;
        else
            cout << fstIter->first << "  =>  " << fstIter->second << endl;

        i++;
    }
}

// Find space based on job size
int findFreeSpace(int jobSize)
{
    int contigous = 0;
    int index;
    bool foundIndex = false;
    map<int, int>::iterator fstIter;

    for (fstIter = FREESPACETABLE.begin(); fstIter != FREESPACETABLE.end(); fstIter++)
    {
        if (fstIter->second == 0)
        {
            contigous++;
            if (!foundIndex)
                index = fstIter->first;
                foundIndex = true;
        }
        else
            continue;
    }

    if (contigous >= jobSize)
    {
        //cout << "contigous: " << contigous << endl;
        return index;
    }

    // if there is no free space
    cout << "findFreeSpace(" << jobSize << "): " << "No free space found!" << endl;
    return -1;
}

void addJobToFST(long jobNumber)
{
    cout << "\n---------------------" << endl;
    cout << "Inside addJobToFST(): " << endl;

    long currJobSize = JOBTABLE[jobNumber-1].getJobSize();
    long currJobAddress = findFreeSpace(currJobSize);
    JOBTABLE[jobNumber-1].SetAddress(currJobAddress);
    JOBTABLE[jobNumber-1].setInMemory(true);

    FREESPACETABLE[currJobAddress] = currJobSize;

    cout << "jobSize: " << currJobSize << endl;
    cout << "jobAddr: " << currJobAddress << endl;
    cout << "FREESPACETABLE[" << currJobAddress << "] = " << currJobSize << endl;
}

int findJob(long jobNum)
{
    for(int i = 0; i < JOBTABLE.size(); i++)
    {
        if(JOBTABLE[i].getJobNumber() == jobNum)
            return i ;
    }

    cout << "findJob(" << jobNum << "): " << "Job not found!" << endl;
    return -1;
}

void clearSpace (int index)
{
    FREESPACETABLE[index] = 0;

}

void clearSpace (int startIndex, int endIndex)
{
    for (int i = startIndex; i < endIndex; i++)
    {
        clearSpace(i);
    }
}

// Handles requests for swapping, starts a drum swap (using SOS function Siodrum), handles drum
// interrupts (Drmint), and selects which job currently on the drum should be swapped into memory.
void swapper(long jobNumber)
{
    int jobLocation = findJob(jobNumber);
    int direction = JOBTABLE[jobLocation].getDirection();

    // direction = 0, swap from drum to memory
    if (direction == 0)
    {
        addJobToFST(jobNumber);
        readyq.push (new int(JOBTABLE[jobLocation].getJobNumber()) );
    }

    // direction = 1, swap from memory to drum
    else if (direction == 1)
    {
        if (JOBTABLE[jobLocation].getIORequest() != true || JOBTABLE[jobLocation].isLatched() != true)
        {
            if (!readyq.empty())
                readyq.pop();

            if (!ioQueue.empty())
                ioQueue.pop();


            siodrum(JOBTABLE[jobLocation].getJobNumber(), JOBTABLE[jobLocation].getJobSize(),
                     JOBTABLE[jobLocation].getAddress(), 1);
            JOBTABLE[jobLocation].setInMemory(false);
            clearSpace(jobLocation);
        }
    }

}
