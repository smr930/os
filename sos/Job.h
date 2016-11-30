#ifndef JOB_H_INCLUDED
#define JOB_H_INCLUDED

class Job
{
    long jobNumber;
    long jobSize;
	long address;
    long timeSlice;
    long direction;
    long timeArrived;
    long ioRequest;
	long priority;

    bool latched;
    bool blocked;
	bool inMemory;
	bool running;
    bool terminated;

public:
    Job()
    {
        jobNumber = -1;
        jobSize = 0;
        address = -1;
        timeSlice = 0;
        direction = 0;
        timeArrived = 0;
        ioRequest = 0;
        priority = 0;

        latched = false;
        blocked = false;
        inMemory = false;
        running = false;
        terminated = false;

    }

    Job (long _jobNumber, long _priority, long _jobSize, long maxCpuTime, long currTime)
    {
        jobNumber = _jobNumber;
        priority = _priority;
        jobSize = _jobSize;
        timeSlice = maxCpuTime;
        timeArrived = currTime;
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

    long getTimeSlice()
    {
        return timeSlice;
    }

    void setTimeSlice (long n)
    {
        timeSlice = n;
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


};

#endif
