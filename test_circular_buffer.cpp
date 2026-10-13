#include "src/core/Utils.h"
#include <cassert>
#include <vector>
int main() {
    bool rejected = false;
    try { circular_buffer<int> invalid(0); } catch (const std::invalid_argument&) { rejected = true; }
    assert(rejected);
    circular_buffer<int> buffer(3);
    buffer.push_back(10); buffer.push_back(20); buffer.push_back(30); buffer.push_back(40);
    assert(buffer[buffer.size() - 2] == 30);
    assert(*std::prev(buffer.end(), 2) == 30);
    auto it = buffer.end(); std::advance(it, -3); assert(*it == 20);
    assert(buffer.front() == 20 && buffer.back() == 40);
    assert(std::vector<int>(buffer.begin(), buffer.end()) == std::vector<int>({20,30,40}));
    buffer.pop_front(); assert(buffer.front() == 30);
    buffer.clear(); assert(buffer.empty());
    buffer.push_back(50); assert(buffer.front() == 50 && buffer.back() == 50);
}
