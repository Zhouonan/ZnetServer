#pragma once

#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>

namespace ZnetServer {
class Semaphore {
public:
    Semaphore(uint32_t count = 0) {
        sem_init(&m_semaphore, 0, count);
    }

    ~Semaphore() {
        sem_destroy(&m_semaphore);
    }

    void wait() {
        sem_wait(&m_semaphore);
    }

    void notify() {
        sem_post(&m_semaphore);
    }
private:
    sem_t m_semaphore;
};

class Mutex {
public:
    Mutex() {
        pthread_mutex_init(&m_mutex, nullptr);
    }
    ~Mutex() {
        pthread_mutex_destroy(&m_mutex);
    }
    void lock() {
        pthread_mutex_lock(&m_mutex);
    }
    void unlock() {
        pthread_mutex_unlock(&m_mutex);
    }
private:
    pthread_mutex_t m_mutex;
};

template<class T>
class ScopedLockImpl {
public: 
    ScopedLockImpl(T mutex) : m_mutex(mutex) {
        m_mutex.lock();
        m_locked = true;
    }

    ~ScopedLockImpl() {
        if (m_locked) {
            m_mutex.unlock();
            m_locked = false;
        }
    }

    void lock() {
        if (!m_locked) {
            m_mutex.lock();
            m_locked = true;
        }
    }

    void unlock() {
        if (m_locked) {
            m_mutex.unlock();
            m_locked = false;
        }
    }
private: 
    T m_mutex;
    bool m_locked;
};

using ScopedLock = ScopedLockImpl<Mutex>;

class NullMutex {
public:
    NullMutex() {}
    ~NullMutex() {}

    void lock() {}
    void unlock() {}   
private:
    // No actual mutex implementation
};
}
