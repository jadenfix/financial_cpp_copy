#pragma once

#include <string>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <vector>
#include <deque>
#include <algorithm>
#include <iterator>

// Simple circular buffer replacement for boost::circular_buffer
template<typename T>
class circular_buffer {
private:
    std::vector<T> buffer_;
    size_t capacity_;
    size_t head_ = 0;
    size_t size_ = 0;

public:
    using value_type = T;
    using iterator = typename std::vector<T>::iterator;
    using const_iterator = typename std::vector<T>::const_iterator;

    explicit circular_buffer(size_t capacity) : capacity_(capacity) {
        buffer_.resize(capacity);
    }

    void push_back(const T& item) {
        buffer_[head_] = item;
        head_ = (head_ + 1) % capacity_;
        if (size_ < capacity_) {
            size_++;
        }
    }

    void push_back(T&& item) {
        buffer_[head_] = std::move(item);
        head_ = (head_ + 1) % capacity_;
        if (size_ < capacity_) {
            size_++;
        }
    }

    size_t size() const { return size_; }
    size_t capacity() const { return capacity_; }
    bool empty() const { return size_ == 0; }
    
    void clear() { 
        size_ = 0; 
        head_ = 0; 
    }
    
    bool full() const { 
        return size_ == capacity_; 
    }
    
    const T& front() const {
        if (size_ == 0) throw std::out_of_range("circular_buffer is empty");
        size_t front_idx = (head_ + capacity_ - size_) % capacity_;
        return buffer_[front_idx];
    }
    
    const T& back() const {
        if (size_ == 0) throw std::out_of_range("circular_buffer is empty");
        size_t back_idx = (head_ + capacity_ - 1) % capacity_;
        return buffer_[back_idx];
    }
    
    void pop_front() {
        if (size_ == 0) return;
        size_--;
    }
    
    const T& operator[](size_t index) const {
        if (index >= size_) throw std::out_of_range("index out of range");
        size_t actual_idx = (head_ + capacity_ - size_ + index) % capacity_;
        return buffer_[actual_idx];
    }

    // Iterator support for range-based loops and algorithms
    class const_circular_iterator {
    private:
        const circular_buffer* buf_;
        size_t index_;
        
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = const T*;
        using reference = const T&;

        const_circular_iterator(const circular_buffer* buf, size_t index) 
            : buf_(buf), index_(index) {}

        reference operator*() const {
            size_t actual_idx = (buf_->head_ + buf_->capacity_ - buf_->size_ + index_) % buf_->capacity_;
            return buf_->buffer_[actual_idx];
        }

        pointer operator->() const { return &(operator*()); }

        const_circular_iterator& operator++() { 
            ++index_; 
            return *this; 
        }

        const_circular_iterator operator++(int) { 
            const_circular_iterator tmp = *this; 
            ++index_; 
            return tmp; 
        }

        bool operator==(const const_circular_iterator& other) const {
            return buf_ == other.buf_ && index_ == other.index_;
        }

        bool operator!=(const const_circular_iterator& other) const {
            return !(*this == other);
        }

        const_circular_iterator operator+(size_t n) const {
            return const_circular_iterator(buf_, index_ + n);
        }

        const_circular_iterator operator-(size_t n) const {
            return const_circular_iterator(buf_, index_ - n);
        }

        ptrdiff_t operator-(const const_circular_iterator& other) const {
            return static_cast<ptrdiff_t>(index_) - static_cast<ptrdiff_t>(other.index_);
        }
    };

    const_circular_iterator begin() const {
        return const_circular_iterator(this, 0);
    }

    const_circular_iterator end() const {
        return const_circular_iterator(this, size_);
    }

    // Support for STL algorithms requiring last N elements
    const_circular_iterator last(size_t n) const {
        if (n > size_) n = size_;
        return const_circular_iterator(this, size_ - n);
    }
};

// Utility function to format time_point for printing (using UTC)
inline std::string formatTimestampUTC(const std::chrono::system_clock::time_point& tp) {
    if (tp == std::chrono::system_clock::time_point::min() || tp == std::chrono::system_clock::time_point::max()) {
        return "N/A";
    }
    std::time_t time = std::chrono::system_clock::to_time_t(tp);
    std::tm utc_tm = *std::gmtime(&time); // Use gmtime for UTC
    std::stringstream ss;
    // Use ISO 8601 like format for clarity
    ss << std::put_time(&utc_tm, "%Y-%m-%d %H:%M:%S UTC");
    return ss.str();
}

// You can add other common utility functions here later