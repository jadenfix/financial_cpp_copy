#include "src/core/Utils.h"
#include <cassert>
#include <iterator>
#include <vector>

int main() {
    bool rejected = false;
    try { circular_buffer<int> invalid(0); }
    catch (const std::invalid_argument&) { rejected = true; }
    assert(rejected);
    circular_buffer<int> values(3);
    for (int value : {1, 2, 3, 4, 5}) values.push_back(value);
    assert(values.size() == 3 && values.front() == 3 && values.back() == 5);
    assert((std::vector<int>(values.begin(), values.end()) == std::vector<int>{3, 4, 5}));
    assert(*std::prev(values.end()) == 5);
    assert(*std::prev(values.end(), 2) == 4);
    auto iterator = values.end();
    std::advance(iterator, -3);
    assert(iterator == values.begin() && *iterator == 3);
    auto copy = iterator++;
    assert(*copy == 3 && *iterator == 4);
    copy = iterator--;
    assert(*copy == 4 && *iterator == 3);
    std::vector<int> reversed;
    for (auto it = std::make_reverse_iterator(values.end());
         it != std::make_reverse_iterator(values.begin()); ++it) reversed.push_back(*it);
    assert((reversed == std::vector<int>{5, 4, 3}));
    values.pop_front();
    assert(*std::prev(values.end(), 2) == 4);
    values.clear();
    assert(values.begin() == values.end());
    values.push_back(9);
    assert(*std::prev(values.end()) == 9);
}
