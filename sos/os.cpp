#include <iostream>
#include <list>
#include <vector>
#include <map>
#include <queue>
#include "Job.h"

using namespace std;

// Data structures and global variables
std::list<std::vector<int> > JOBTABLE;
//std::vector<Job> JOBTABLE;
std::map<int, int> FREESPACETABLE; // address and size pair
std::queue<int*> readyq;


void siodisk(int jobnum);
void initFST();

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
 // a = 7 => job wants to be blocked until all its pending
 // I/O requests are completed

}

// Create 100 elements in FST
void initFST()
{
    for (int i = 1; i <= 100; i++)
    {
        FREESPACETABLE[i] = 0;

    }
}

void addJob (int jobnum, int jobsize, int coreaddress, int direction)
{
    // Place the parameters in a temp vector
    std::vector<int> newJob;
    newJob.push_back (jobnum);
    newJob.push_back (jobsize);
    newJob.push_back (coreaddress);
    newJob.push_back (direction);

    // Add the temp vector to the JOBTABLE
    JOBTABLE.push_back(newJob);

}

int findFreeSpace()
{
    for (int i = 1; i <= FREESPACETABLE.size(); i++)
    {
        if (FREESPACETABLE[i] == 0)
        {
            return i;
        }
    }

    // if there is no free space
    return -1;
}

void clearSpace(int index)
{
    FREESPACETABLE[index] = 0;
}

void swapper()
{



}
