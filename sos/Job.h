#ifndef JOB_H_INCLUDED
#define JOB_H_INCLUDED

class Job
{
    long jobNumber;
    long jobSize;
	long address;
    long maxCPUtime;
    long direction;
    long timeArrived;
    long ioRequest;
	long priority;

    bool latched;
    bool blocked;
	bool inMemory;
	bool running;
    bool terminated;
    bool set_to_terminate;
    bool isDoingIO;

  public:
    //default constructor
    Job()
    {
        jobNumber = -1;
        jobSize = 0;
        //FST memory starts at 0 and all jobs start outside of memory
        address = -1;
        maxCPUtime = 0;
        direction = 0;
        timeArrived = 0;
        ioRequest = 0;
        priority = 0;

        latched = false;
        blocked = false;
        inMemory = false;
        running = false;
        terminated = false;
        set_to_terminate = false;
        isDoingIO = false;

    }


    Job (long jobNumber, long jobSize, long maxCPUtime, long timeArrived, long priority)
    {
        this->jobNumber = jobNumber;
        this->jobSize = jobSize;
        this->address = -1;
        this->maxCPUtime = maxCPUtime;
        this->timeArrived = timeArrived;
        this->ioRequest = 0; //start with no I/O requests
        this->priority = priority;
        this->direction = 0;

        this->latched = false;
        this->blocked = false;
        this->inMemory = false;
        this->running = false;
        this->terminated = false;
        this->set_to_terminate = false;
        this->isDoingIO = false;
    }

    long getJobNumber()
    {
        return jobNumber;
    }

    void setJobNumber (long n)
    {
        jobNumber = n;
    }

    long getAddress()
    {
        return address;
    }

    void SetAddress (long n)
    {
        address = n;
    }

    long getJobSize()
    {
        return jobSize;
    }

    void setJobSize (long n)
    {
        jobSize = n;
    }

    long getmaxCPUtime()
    {
        return maxCPUtime;
    }

    void setmaxCPUtime (long n)
    {
        maxCPUtime = n;
    }

    long getDirection()
    {
        return direction;
    }

    void setDirection (long n)
    {
        direction = n;
    }

    long getTimeArrived()
    {
        return timeArrived;
    }

    void setTimeArrived (long n)
    {
        timeArrived = n;
    }

    long getIORequest()
    {
        return ioRequest;
    }

    void setIORequest (long n)
    {
        ioRequest = n;
    }

    long getPriority()
    {
        return priority;
    }

    void setPriority (long n)
    {
        priority = n;
    }

    bool isLatched()
    {
        return latched;
    }

    void setLatched (bool n)
    {
        latched = n;
    }

    bool isBlocked()
    {
        return blocked;
    }

    void setBlocked (bool n)
    {
        blocked = n;
    }

    bool isInMemory()
    {
        return inMemory;
    }

    void setInMemory (bool n)
    {
        inMemory = n;
    }

    bool isRunning()
    {
        return running;
    }

    void setRunning (bool n)
    {
        running = n;
    }

    bool isTerminated()
    {
        return terminated;
    }

    void setTerminated (bool n)
    {
        terminated = n;
    }

    bool is_SetToTerminated()
    {
        return set_to_terminate;
    }

    void setset_to_terminate (bool n)
    {
        set_to_terminate = n;
    }

    bool getIsDoingIO(){
        return isDoingIO;
    }

    void setIsDoingIO(bool n){
        isDoingIO = n;
    }

};

#endif
