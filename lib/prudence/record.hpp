#pragma once

#include <string>
#include <sstream>
#include <vector>

#include <combinations.hpp>

namespace prudence {
    /**
     * @brief Record of the dataset.
     */
    class Record {
    public:
        // String used as an unique ID
        std::string id;
        // Vector of features
        std::vector<float> features;

        /**
         * @brief Construct a new record object
         * 
         * @param row CSV string to parse.
         * @param id_index Index of the column representing the ID, starting from 0.
         * @param _eps Floating epsilon for the match. Defaults to 0.3.
         */
        Record(std::string& row, const int& id_index) {
            std::stringstream stream(row);
            std::string column;
            int j = 0;
            while (std::getline(stream, column, ',')) {
                if (j == id_index)
                    id = column;
                else {
                    float value = strtof(column.c_str(), NULL);
                    features.push_back(value);
                }
                j++;
            }
        }
        /**
         * @brief Destroy the record object.
         * 
         */
        ~Record() {}

        /**
         * @brief Matches a record agaist another
         * 
         * @param v Other record to match
         * @param eps Epsilon margin of the computation
         * @param mask Mask that signals what indices are accountable to the match.
         * @return true If all the selected indices match.
         * @return false Otherwise.
         */
        bool matches(const Record& v, const float& eps, std::vector<bool> mask) {
            for (size_t j = 0; j < mask.size(); j++)
                if (mask[j]) {
                    float lo = v.features[j] - v.features[j] * eps;
                    float hi = v.features[j] + v.features[j] * eps;
                    if (this->features[j] < lo || this->features[j] > hi)
                        return false;
                }
            return true;
        }
    };
} // namespace prudence