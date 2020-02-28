//
//  Thread.h
//  eCash_Bank
//
//  Created by Raisa Islam on 18/2/20.
//  Copyright © 2020 Raisa Islam. All rights reserved.
//
#ifndef Thread_h
#define Thread_h

#include <thread>
#include <chrono>
#include <string>
#include <pthread.h>

class Thread {
public:
    Thread();
    virtual ~Thread();
    
    void start();
    void stop();
    void join();
    void detach();
    bool isRunning();
    
    std::thread::native_handle_type getNativeHandle();
    
private:
    bool running;
    std::thread t;
    
    std::thread spawn();
    
protected:
    virtual void runner() = 0;
};


#endif /* Thread_h */
