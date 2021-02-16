#pragma once
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>

template<class T>
class SafeQueue {
    std::queue<T*> q;
    std::mutex mtx;
    std::condition_variable cv;

public:

    SafeQueue() {}
    int count ()
    {
        return q.size();
    }

    void push(T* elem) {
        if(elem == nullptr) {
            return;
        }
        std::unique_lock<std::mutex> lck(mtx);
        q.push(elem);
        cv.notify_one();
    }

    T* pop() {
        T* elem = nullptr;

        std::unique_lock<std::mutex> lck(mtx);
        //cv.wait(lck, !q.empty());
        cv.wait(lck, [&](){return !q.empty();});
            
        if(!q.empty()) {
            elem = q.front();
            q.pop();
        }
    
        return elem;
    }

};