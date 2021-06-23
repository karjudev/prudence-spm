#pragma once

#include <vector>

#include <safe_queue.hpp>
#include <prudence/utils.hpp>

/**
 * @brief Chunk of indices in an array
 */
struct chunk_t {
    size_t begin;
    size_t end;
};

/**
 * @brief Alias name of the thread
 * 
 */
using thread_id = short;


/**
 * @brief Emitter that dinamically assigns slices to the workers via queues and feedback queue.
 * 
 * @param queues Queues from the emitter to each worker.
 * @param feedback_queue Feedback queue from the workers to the emitter.
 * @param n Number of records in the dataset.
 */
void emitter(
    std::vector<SafeQueue<std::optional<chunk_t>>>& queues,
    SafeQueue<thread_id>& feedback_queue,
    const size_t& n
) {
    // Number of workers in the dataset
    short nw = queues.size();
    // Decides the chunk size (half of n/nw)
    size_t chunk_size{n / (2 * nw)};
    // Begin of the current chunk
    size_t begin = 0;
    // End of the current chunk
    size_t end = std::min(chunk_size, n);
    // Assigns the first chunks
    for (thread_id i = 0; i < nw; i++) {
        chunk_t chunk = { begin, end};
        queues[i].push(std::move(chunk));
        begin = end;
        end = std::min(begin + chunk_size, n);
    }
    // Waits for requests from the users
    while (nw > 0) {
        // Gets a new thread ID
        thread_id tid = feedback_queue.pop();
        // If begin is equal to n we don't have any more chunks to dispatch
        if (begin == n) {
            // Pushes the EOS
            queues[tid].push({});
            // Decrements the number of active workers
            nw--;
        }
        else {
            // Chunk to send into the queue
            chunk_t chunk = { begin, end };
            // Sends data into the queue
            queues[tid].push(std::move(chunk));
            // Increments begin
            begin = end;
            // Increments end based on the new value of begin
            end = std::min(begin + chunk_size, n);
        }
    }
}

/**
 * @brief Worker that computes risks on the given chunks.
 * 
 * @param dataset Global view of the dataset.
 * @param risk_vector Vector in wich to put the risk values.
 * @param queue Queue to get chunks.
 * @param feedback_queue Feedback queue to request new chunks.
 * @param h Background knowledge size.
 * @param eps Margin for the matching computation.
 */
void worker(
    const thread_id& id,
    std::vector<prudence::Record>& dataset,
    std::vector<float>& risk_vector,
    SafeQueue<std::optional<chunk_t>>& queue,
    SafeQueue<thread_id>& feedback_queue,
    const short& h,
    const float& eps
) {
    // Flag that signals that the thread is running
    bool running = true;
    while (running) {
        // Gets the chunk
        std::optional<chunk_t> data = queue.pop();
        // If an EOS is received, terminate
        if (!data)
            running = false;
        else {
            // Chunk to compute
            chunk_t chunk = data.value();
            // Computes the risk on a chunk
            for (size_t i = chunk.begin; i < chunk.end; i++) {
                float risk = prudence::assess_risk(dataset[i], dataset, h, eps);
                risk_vector[i] = risk;
            }
            // Requests the next chunk
            feedback_queue.push(id);
        }
    }
}