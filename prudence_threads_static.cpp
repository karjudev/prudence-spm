#include <iostream>
#include <thread>

#include <prudence/utils.hpp>

#include <utimer.hpp>

/**
 * @brief Computes the risk for a given range of records in the dataset.
 * 
 * @param dataset Global view of the dataset.
 * @param begin Starting index of the slice.
 * @param end Ending index of the slice.
 * @param h Background knowledge size.
 * @param eps Epsilon margin for the matching.
 * @param risk_vector Vector to put the risk values.
 */
void worker(
    std::vector<prudence::Record>& dataset,
    const size_t& begin,
    const size_t& end,
    const short& h,
    const float& eps,
    std::vector<float>& risk_vector
) {
    for (size_t i = begin; i < end; i++) {
        prudence::Record& u = dataset[i];
        float risk = prudence::assess_risk(u, dataset, h, eps);
        risk_vector[i] = risk;
    }
}

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
    float eps = (argc == 6) ? strtof(argv[3], NULL) : 0.3;
    // If provided, id_index
    int id_index = (argc == 7) ? strtol(argv[4], NULL, 10) : 0;
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
        // Vector of threads
        std::vector<std::thread> workers(nw);
        // Dimension of a chunk assigned to a thread
        size_t chunk_size{n/nw};
        // Timer
        UTimer timer(&comp_time);
        for (short i = 0; i < nw; i++) {
            // Starting index of the chunk
            size_t begin = i * chunk_size;
            // Ending index of the chunk
            size_t end = (i != (nw-1)) ? ((i + 1) * chunk_size) : n;
            // Spans a new worker thread
            std::thread w(worker, std::ref(dataset), std::move(begin), std::move(end), h, eps, std::ref(risk_vector));
            workers[i] = std::move(w);
        }
        // Joins all the workers
        for (std::thread& w: workers)
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
