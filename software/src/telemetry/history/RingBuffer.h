#pragma once

#include <array>
#include <cstddef>

namespace eclipse::telemetry::history {

    //fixed-size circular buffer
    //when full, new pushes overwrite the oldest element
    template <typename T, std::size_t Capacity>

    //stores the last N points efficiently
    class RingBuffer {
    public:
        RingBuffer() = default;

        //removes all elements from the buffer
        void clear() {
            _head = 0;
            _size = 0;
        }

        //returns how many elements are currently stored
        std::size_t size() const {
            return _size;
        }

        //returns the maximum number of elements the buffer can store
        constexpr std::size_t capacity() const {
            return Capacity;
        }

        //returns true if there are no elements
        bool empty() const {
            return _size == 0;
        }

        //returns true if the buffer is full
        bool full() const {
            return _size == Capacity;
        }

        //pushes a new element.
        //if full, overwrites the oldest element
        void push(const T& v) {
            _data[_head] = v;
            _head = (_head + 1) % Capacity;

            if (_size < Capacity) {
                _size++;
            }
        }

        //access element by age (0 = oldest, size-1 = newest)
        const T& at(std::size_t i) const {
            //oldest element index depends on head and size
            const std::size_t start = (_head + Capacity - _size) % Capacity;
            const std::size_t idx = (start + i) % Capacity;
            return _data[idx];
        }

        //non-const access (same ordering)
        T& at(std::size_t i) {
            const std::size_t start = (_head + Capacity - _size) % Capacity;
            const std::size_t idx = (start + i) % Capacity;
            return _data[idx];
        }

    private:
        std::array<T, Capacity> _data{}; //underlying storage for the elements
        std::size_t _head = 0;           //next write position
        std::size_t _size = 0;           //number of valid elements in buffer
    };

}