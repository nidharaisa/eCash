//
//  Thread.cpp
//  eCash_Bank
//
//  Created by Raisa Islam on 18/2/20.
//  Copyright © 2020 Raisa Islam. All rights reserved.
//

#include "Thread.h"

Thread::Thread() {
    running = false;
}

Thread::~Thread() {
}

void Thread::start()
{
    running = true;
    t = spawn();
}

std::thread::native_handle_type Thread::getNativeHandle()
{
    return t.native_handle();
}

void Thread::stop()
{
    running = false;
}

bool Thread::isRunning()
{
    return running;
}

void Thread::join()
{
    if (t.joinable())
    {
        t.join();
    }
}

void Thread::detach()
{
    t.detach();
}

std::thread Thread::spawn()
{
    return std::thread(&Thread::runner, this);
}

