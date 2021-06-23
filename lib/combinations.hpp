#pragma once

#include <algorithm>
#include <vector>

/**
 * @brief Iterator that constructs a bitmask corresponding to every combination of k numbers in {1 ... n}
 * 
 */
class CombinationsEnumerator {
public:
    // Bitvector representing the items
    std::vector<bool> mask;
    // Index of the current combination
    int i;
    
    /**
     * @brief Construct a new combinations object
     * 
     * @param n Total number of items.
     * @param k Size of the combination.
     */
    CombinationsEnumerator(int n, int k) {
        // k trailing ones
        mask = std::vector<bool>(k, 1);
        // n - k other zeros
        mask.resize(n, 0);
        i = 0;
    }
    /**
     * @brief Destroy the combinations object
     */
    ~CombinationsEnumerator() {}

    /**
     * @brief Gets the next permutation
     * 
     * @return true If the permutation is not the last one.
     * @return false If the permutation is the last one.
     */
    bool next() {
        i++;
        return std::prev_permutation(mask.begin(), mask.end());
    }
};