#include <iostream>

#include <ff/ff.hpp>
#include <ff/parallel_for.hpp>

#include <prudence/utils.hpp>

#include <utimer.hpp>

using namespace ff;

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
    // Computation time
    float compute_time;
    // If nw is 0 performs the sequential algorithm
    if (nw == 0) {
        ffTime(START_TIME);
        sequential_algorithm(std::ref(dataset), h, eps, std::ref(risk_vector));
        compute_time = ffTime(STOP_TIME);
    }
    else {
        // Chunk size is half of n / nw
        size_t chunk_size{n / (2 * nw)};
        // Parallel for executor
        ParallelFor pf(nw);
        ffTime(START_TIME);
        pf.parallel_for(0, n, 1, chunk_size, [&dataset, &risk_vector, &h, &eps](const long& i) {
            // User taken into account
            prudence::Record& u = dataset[i];
            // Risk for the user
            float risk = prudence::assess_risk(u, dataset, h, eps);
            // Puts the risk in the output vector
            risk_vector[i] = risk;
        }, nw);
        compute_time = ffTime(STOP_TIME);
    }
    // Output stream
    std::ofstream output_stream(argv[4]);
    if (!output_stream.is_open()) {
        std::cerr << argv[0] << " was unable to open output file " << argv[4] << std::endl;
        return EXIT_FAILURE;
    }
    // Writes risk vector on disk
    prudence::write_risk(std::ref(dataset), std::ref(risk_vector), std::ref(output_stream));
    output_stream.close();
    std::cout << "Time: " << compute_time << std::endl;
    return 0;
}
