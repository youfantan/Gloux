#include <iostream>
#include "gloux/io.h"

namespace gloux::io {
    ByteBuffer::ByteBuffer() : capacity_(single_expand_size), position_(0), positive_order_(true), little_endian_(true), self_allocated_(true) {
        ptr_ = new char [single_expand_size];
        memset(ptr_, 0, single_expand_size);
    }

    ByteBuffer::ByteBuffer(uint64_t capacity) : capacity_(capacity), position_(0), positive_order_(true), little_endian_(true), self_allocated_(true) {
        ptr_ = new char [capacity];
        memset(ptr_, 0, capacity);
    }

    ByteBuffer::ByteBuffer(char *ptr, uint64_t size, bool copy) : capacity_(size), position_(0), positive_order_(true), little_endian_(true), self_allocated_(true) {
        if (copy) {
            ptr_ = new char [size];
            memcpy(ptr_, ptr, size);
        } else {
            ptr_ = ptr;
        }
    }

    ByteBuffer::ByteBuffer(const ByteBuffer &buffer) {
        ptr_ = new char [buffer.capacity_];
        memcpy(ptr_, buffer.ptr_, buffer.capacity_);
        capacity_ = buffer.capacity_;
        position_ = buffer.position_;
        positive_order_ = buffer.positive_order_;
        little_endian_ = buffer.little_endian_;
        self_allocated_ = true;
    }

    ByteBuffer::~ByteBuffer() {
        std::cout << "DEBUG: ByteBuffer destructed" << std::endl;
        if (self_allocated_) {
            delete[] ptr_;
        }
    }

    ByteBuffer ByteBuffer::read(const uint64_t begin, const uint64_t end) const {
        {
            std::unique_ptr<std::mutex> lock;
            return {ptr_ + begin, end - begin};
        }
    }

    void ByteBuffer::write(const uint64_t begin, const ByteBuffer &&buffer) {
        {
            std::unique_ptr<std::mutex> lock;
            if (begin + buffer.capacity_ > capacity_) {
                expand((begin + buffer.capacity_) - capacity_);
            }
            memcpy(ptr_ + begin, buffer.ptr_, buffer.capacity_);
        }
    }

    ByteBuffer ByteBuffer::read(const uint64_t size) {
        {
            std::unique_ptr<std::mutex> lock;
            if (positive_order_) {
                auto buffer = ByteBuffer(ptr_ + position_, size);
                position_ += size;
                return buffer;
            } else {
                position_ -= size;
                return {ptr_ + position_, size};
            }
        }
    }

    void ByteBuffer::write(const ByteBuffer &&buffer) {
        {
            std::unique_ptr<std::mutex> lock;
            write(position_, std::forward<const ByteBuffer>(buffer));
            position_ += buffer.capacity_;
        }
    }

    uint64_t ByteBuffer::capacity() const {
        return capacity_;
    }

    void ByteBuffer::capacity(const uint64_t value) {
        capacity_ = value;
    }

    uint64_t ByteBuffer::position() const {
        return position_;
    }

    void ByteBuffer::position(const uint64_t value) {
        position_ = value;
    }

    bool ByteBuffer::positive_order() const {
        return positive_order_;
    }

    void ByteBuffer::positive_order(const bool value) {
        positive_order_ = value;
    }

    bool ByteBuffer::little_endian() const {
        return little_endian_;
    }

    void ByteBuffer::little_endian(const bool value) {
        little_endian_ = value;
    }

    bool ByteBuffer::self_allocated() const {
        return self_allocated_;
    }

    void ByteBuffer::expand(const uint64_t size) {
        char *ptr = new char [capacity_ + size];
        memcpy(ptr, ptr_, capacity_);
        delete[] ptr_;
        ptr_ = ptr;
        capacity_ = capacity_ + size;
    }

    void ByteBuffer::expand() {
        expand(single_expand_size);
    }

    char& ByteBuffer::operator[](uint64_t position) {
        return ptr_[position];
    }

    ByteBuffer::operator int() {
        return *(reinterpret_cast<int *>(ptr_));
    }

    ByteBuffer::operator long() {
        return *(reinterpret_cast<long *>(ptr_));
    }

    ByteBuffer::operator short() {
        return *(reinterpret_cast<short *>(ptr_));
    }

    ByteBuffer::operator long long() {
        return *(reinterpret_cast<long long *>(ptr_));
    }

    ByteBuffer::operator unsigned int() {
        return *(reinterpret_cast<unsigned int *>(ptr_));
    }

    ByteBuffer::operator unsigned long() {
        return *(reinterpret_cast<unsigned long *>(ptr_));
    }

    ByteBuffer::operator unsigned short() {
        return *(reinterpret_cast<unsigned short *>(ptr_));
    }

    ByteBuffer::operator unsigned long long() {
        return *(reinterpret_cast<unsigned long long *>(ptr_));
    }

    ByteBuffer::operator float() {
        return *(reinterpret_cast<float *>(ptr_));
    }

    ByteBuffer::operator double() {
        return *(reinterpret_cast<double *>(ptr_));
    }

    ByteBuffer::operator std::string() {
        return {ptr_, capacity_};
    }

}