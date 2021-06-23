#pragma once

#include <optional>
#include <queue>
#include <mutex>
#include <condition_variable>

/**
 * @brief Thread safe queue
 * 
 * @tparam T Type of the items passed into the queue
 */
template <typename T>
class SafeQueue {
private:
    std::mutex mut;
    std::condition_variable cond;
    std::queue<T> unsafe_queue;
public:
    /**
     * @brief Construct a new Safe Queue object
     */
    SafeQueue() {}
    
    /**
     * @brief Destroy the Safe Queue object
     */
    ~SafeQueue() {}

    /**
     * @brief Pushes a new value in the queue.
     * 
     * @param value Value to push in the queue.
     */
    void push(T const& value) {
        {
            // Locks the access to the queue
            std::unique_lock<std::mutex> lock(mut);
            // Pushes the value in the unsafe queue
            unsafe_queue.push(value);
        }
        // Notifies the listeners that one value has been added
        cond.notify_one();
    }

    /**
     * @brief Waits until the queue is non-empty, then gets an element.
     * 
     * @return T Element popped from the queue.
     */
    T pop() {
        // Locks the access to the queue
        std::unique_lock<std::mutex> lock(mut);
        // Waits until the queue is non-empty
        cond.wait(lock, [=]{ return !unsafe_queue.empty(); });
        // Gets the result
        T result(std::move(unsafe_queue.front()));
        // Removes it from the queue
        unsafe_queue.pop();
        return result;
    }
};