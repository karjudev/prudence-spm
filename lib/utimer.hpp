#pragma once

#include <chrono>

/**
 * @brief Timer using RAII
 * 
 */
class UTimer {
private:
    using usecs = std::chrono::microseconds;
    using msecs = std::chrono::milliseconds;
    // Starting point in time
    std::chrono::system_clock::time_point start;
    // Stopping point in time
    std::chrono::system_clock::time_point stop;
    // Pointer to microseconds elapsed to save state
    long* elapsed;
public:
    /**
     * @brief Construct a new UTimer object.
     * 
     * @param _message Message to print at the stopping time.
     * @param _elapsed Pointer to save the number of microseconds elapsed.
     */
    UTimer(long* _elapsed): elapsed(_elapsed) {
        start = std::chrono::system_clock::now();
    }

    /**
     * @brief Destroy the UTimer object and print the message and time elapsed.
     * 
     */
    ~UTimer() {
        stop = std::chrono::system_clock::now();
        std::chrono::duration<double> time_elapsed = stop - start;
        auto musec = std::chrono::duration_cast<std::chrono::microseconds>(time_elapsed).count();
        if (elapsed != NULL)
            *elapsed = musec;
    }
};