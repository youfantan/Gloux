#pragma once

#include <cstdint>
#include <memory>
#include <cstring>

constexpr int DEFAULT_AUTO_RESIZE_LENGTH = 128 * 1024;

namespace gloux::io {
    class ByteBuffer {
        static constexpr uint64_t single_expand_size = 16 * 1024;
    private:
        char *ptr_;
        uint64_t capacity_;
        uint64_t position_;
        bool positive_order_;
        bool little_endian_;
        bool self_allocated_;
        std::mutex mtx_;
    public:
        ByteBuffer();
        template<class T>
        ByteBuffer(T & data) : ByteBuffer() {
            delete[] ptr_;
            self_allocated_ = false;
            ptr_ = &data;
            capacity_ = sizeof(data);
        }
        ByteBuffer(std::string & data) : ByteBuffer() {
            delete[] ptr_;
            self_allocated_ = false;
            ptr_ = &data[0];
            capacity_ = data.size();
        }
        template<class T>
        ByteBuffer(T && data) : ByteBuffer() {
            delete[] ptr_;
            capacity_ = sizeof(data);
            ptr_ = new char [capacity_];
            memcpy(ptr_, &data,capacity_);
        }
        ByteBuffer(std::string && data) : ByteBuffer() {
            delete[] ptr_;
            capacity_ = data.size();
            ptr_ = new char [capacity_];
            memcpy(ptr_, &data[0],capacity_);
        }

        explicit ByteBuffer(uint64_t capacity);
        ByteBuffer(char *ptr, uint64_t size, bool copy = true);
        ByteBuffer(const ByteBuffer & buffer);
        ~ByteBuffer();
        ByteBuffer read(const uint64_t begin, const uint64_t end) const;
        void write(const uint64_t begin, const ByteBuffer && buffer);
        ByteBuffer read(const uint64_t size);
        void write(const ByteBuffer && buffer);
        template<class T>
        T read(const uint64_t begin) {
            if (positive_order_) {
                T t;
                memcpy(&t, ptr_ + begin, sizeof(T));
                return t;
            } else {
                T t;
                memcpy(&t, ptr_ + begin - sizeof(T), sizeof(T));
                return t;
            }
        }
        template<class T>
        T read() {
            if (positive_order_) {
                T t;
                memcpy(&t, ptr_ + position_, sizeof(T));
                position_ += sizeof(T);
                return t;
            } else {
                T t;
                memcpy(&t, ptr_ + position_ - sizeof(T), sizeof(T));
                position_ -= sizeof(T);
                return t;
            }
        }
        template<class T>
        void write(const uint64_t begin, const T && data) {
            if (positive_order_) {
                memcpy(ptr_ + begin, &data, sizeof(T));
            } else {
                memcpy(ptr_ + begin - sizeof(T), &data, sizeof(T));
            }
        }
        template<class T>
        void write(const T && data) {
            if (positive_order_) {
                memcpy(ptr_ + position_, &data, sizeof(T));
                position_ += sizeof(T);
            } else {
                memcpy(ptr_ + position_ - sizeof(T), &data, sizeof(T));
                position_ -= sizeof(T);
            }
        }
        uint64_t capacity() const;
        void capacity(const uint64_t value);
        uint64_t position() const;
        void position(const uint64_t value);
        bool positive_order() const;
        void positive_order(const bool value);
        bool little_endian() const;
        void little_endian(const bool value);
        bool self_allocated() const;
        void expand(const uint64_t capacity);
        void expand();
        char & operator[] (uint64_t position);
        operator int();
        operator long();
        operator short();
        operator long long();
        operator unsigned int();
        operator unsigned long();
        operator unsigned short();
        operator unsigned long long();
        operator float();
        operator double();
        operator std::string();
    };
}