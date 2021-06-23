#include <iostream>
#include <thread>

#include <entities.hpp>

#include <utimer.hpp>

int main(int argc, char const *argv[]) {
    // Checks the CLI parameters size
    if (argc < 5) {
        std::cerr << "Usage: " << argv[0] << " nw h input_filename output_filename [eps=0.3] [id_index=0]" << std::endl;
        return EXIT_FAILURE;
    }
    // Number of workers to use
    short nw = (short) strtol(argv[1], NULL, 10);
    // Background knowledge size
    short h = (short) strtol(argv[2], NULL, 10);
    // If provided, epsilon margin
    float eps = (argc == 6) ? strtof(argv[5], NULL) : 0.3;
    // If provided, id_index
    int id_index = (argc == 7) ? strtol(argv[6], NULL, 10) : 0;
    // Input stream
    std::ifstream input_stream(argv[3]);
    if (!input_stream.is_open()) {
        std::cerr << argv[0] << " was unable to open input file " << argv[3] << std::endl;
        return EXIT_FAILURE;
    }
    // Reads the dataset
    std::vector<prudence::Record> dataset = prudence::read_dataset(input_stream, id_index);
    // Closes the input stream
    input_stream.close();
    // Number of records in the dataset
    size_t n = dataset.size();
    // Array of risk values
    std::vector<float> risk_vector(n);
    // Time spent in the computation phase
    long comp_time;
    // If nw is 0 performs the sequential algorithm
    if (nw == 0) {
        UTimer timer(&comp_time);
        sequential_algorithm(std::ref(dataset), h, eps, std::ref(risk_vector));
    }
    else {
        // Vector of queues
        std::vector<SafeQueue<std::optional<chunk_t>>> queues(nw);
        // Feedback queue
        SafeQueue<thread_id> feedback_queue;
        // Vector of threads
        std::vector<std::thread> worker_threads(nw);
        // Timer that cronometrates the latency
        UTimer timer(&comp_time);
        for (thread_id i = 0; i < nw; i++) {
            // Worker thread
            std::thread worker_thread(
                worker,
                i,
                std::ref(dataset),
                std::ref(risk_vector),
                std::ref(queues[i]),
                std::ref(feedback_queue),
                h,
                eps
            );
            worker_threads[i] = std::move(worker_thread);
        }
        // To spare a thread, the main thread becomes the emitter
        emitter(std::ref(queues), std::ref(feedback_queue), n);
        // Joins all the entities
        for (std::thread& w: worker_threads)
            w.join();
    }
    // Output stream
    std::ofstream output_stream(argv[4]);
    if (!output_stream.is_open()) {
        std::cerr << argv[0] << " was unable to open output file " << argv[4] << std::endl;
        return EXIT_FAILURE;
    }
    prudence::write_risk(std::ref(dataset), std::ref(risk_vector), std::ref(output_stream));
    output_stream.close();
    std::cout << "Time: " << comp_time / 1000.0 << std::endl;
    return 0;
}
