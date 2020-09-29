# Note

## condition_variable

    wait(lock,predictiton):
        // assume lock is acquired
        while (!predictiton()) {    //check condition
            wait(lock);             //atomic "wait and unlock"
        // lock is reacquired before wait exits
        } 

## unique_lock

    unique_lock(mutex):
        // lock
    ~unique_lock():
        // release unlock if mutex is acquired
