#pragma once

#include <climits>
#include <fstream>
#include <vector>

#include <prudence/record.hpp>

namespace prudence {
    /**
     * @brief Reads the dataset from an input stream
     * 
     * @param input_stream Input stream to read the data
     * @param id_index Index of the column that has to be accounted as ID for the record.
     * @return std::vector<Record> Vector of records (dataset)
     */
    inline std::vector<Record> read_dataset(std::ifstream& input_stream, const int& id_index) {
        std::vector<Record> dataset;
        std::string row;
        // Skips the first line (header)
        std::getline(input_stream, row);
        // Reads the whole dataset
        while (std::getline(input_stream, row)) {
            Record u(row, id_index);
            dataset.push_back(u);
        }
        return dataset;
    }

    /**
     * @brief Writes the risk vector to the output stream.
     * 
     * @param dataset Dataset to read the usernames.
     * @param risks Vector of risks.
     * @param output_stream Stream to write data on a file.
     */
    inline void write_risk(const std::vector<Record>& dataset, const std::vector<float>& risks, std::ofstream& output_stream) {
        // Writes the header
        output_stream << "ID,Risk" << std::endl;
        for (size_t i = 0; i < dataset.size(); i++)
            output_stream << dataset[i].id << "," << risks[i] << std::endl;
    }

    /**
     * @brief Computes the number of matches for the record u in the dataset.
     * 
     * @param u User's record.
     * @param dataset Global view of the dataset.
     * @param h Background knowledge size.
     * @param eps Margin of the matching.
     * @param mask Boolean mask representing the indices that have to be taken into account.
     * @return int Number of matches of u.
     */
    static int matches_combination(
        Record& u,
        const std::vector<Record>& dataset,
        const float& eps,
        const std::vector<bool>& mask
    ) {
        int matches = 0;
        for (const Record& v: dataset)
            matches += u.matches(v, eps, mask);
        return matches;
    }

    /**
     * @brief Assesses the risk of a record in a dataset
     * 
     * @param u User's record.
     * @param dataset Global view of the dataset.
     * @param h Background knowledge size.
     * @param eps Epsilon margin for the match.
     * @return float Risk for the user.
     */
    float assess_risk(
        Record& u,
        const std::vector<Record>& dataset,
        const short& h,
        const float& eps
    ) {
        // Minimum number of matches for a combination
        int min_matches = INT_MAX;
        CombinationsEnumerator comb(u.features.size(), h);
        do {
            // Number of matches for the combination
            int matches = matches_combination(u, dataset, eps, comb.mask);
            // If we have only 1 match the combination gives the risk
            if (matches == 1) {
                return 1.0;
            }
            // Else we take the minimum number of matches found
            if (matches < min_matches)
                min_matches = matches;
        } while (comb.next());
        // Risk is the inverse of the minimum number of matches
        return 1.0 / min_matches;
    }

    /**
     * @brief Computes the PRUDEnce algorithm in a sequential manner.
     * 
     * @param dataset Reference to the full dataset.
     * @param h Background knowledge size.
     * @param eps Epsilon margin for the matching.
     * @param risk_vector Vector to store the risk values.
     */
    void sequential_algorithm(std::vector<Record>& dataset, const short& h, const float& eps, std::vector<float>& risk_vector) {
        for (size_t i = 0; i < dataset.size(); i++) {
            float risk = assess_risk(dataset[i], dataset, h, eps);
            risk_vector[i] = risk;
        }
    }
} // namespace prudence