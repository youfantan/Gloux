#pragma once

#include <cstdint>
#include <memory>
#include <cstring>
#include <utility>
#include <cassert>
#include <functional>

constexpr int BYTEBUFFER_UINT_SIZE = 128 * 1024;

namespace gloux::io {
    class bytebuffer_exception : public std::exception {
    private:
        const std::string msg_;
    public:
        explicit bytebuffer_exception(std::string msg) : msg_(std::move(msg)) {}
        const char *what() {
            return msg_.c_str();
        }
    };
    class bytebuffer {
        using callback_t = std::function<void(uint64_t, uint64_t)>;
    private:
        uint64_t position_{};
        uint64_t limit_{};
        uint64_t capacity_{};
        char *data_{};
        bool read_order_{false}; // reverse read
        bool write_order_{true}; // forward read
        bool byte_order_{true}; // little endian
        bool can_free_ptr_{true}; // self-allocated memory
        callback_t read_callback_;
        callback_t write_callback_;
    public:
        bytebuffer() = default;
        explicit bytebuffer(uint64_t size) : limit_(size), capacity_(size){
            data_ = new char[size];
            memset(data_, 0, size);
        }
        bytebuffer(char *buffer, uint64_t size, bool copy = true) : can_free_ptr_(copy), limit_(size), capacity_(size) {
            if (copy) {
                data_ = new char[size];
                memcpy(data_, buffer, size);
            } else {
                // not safe
                data_ = buffer;
            }
            can_free_ptr_ = copy;
            limit_ = size;
            capacity_ = size;
        }
        bytebuffer(const bytebuffer &buffer) : bytebuffer(buffer.data_, buffer.limit_, true){
            read_order_ = buffer.read_order_;
            write_order_ = buffer.write_order_;
            byte_order_ = buffer.byte_order_;
        }
        bytebuffer(const bytebuffer &&buffer) noexcept : bytebuffer(buffer.data_, buffer.limit_, true) {
            read_order_ = buffer.read_order_;
            write_order_ = buffer.write_order_;
            byte_order_ = buffer.byte_order_;
        }
        explicit bytebuffer(std::string &buffer, bool copy = true) : bytebuffer(reinterpret_cast<char *>(&buffer[0]), buffer.size(), copy){}
        ~bytebuffer() {
            if (can_free_ptr_) {
                delete[] data_;
            }
        }
        void *operator new(unsigned long) = delete;
        void *operator new[](unsigned long) = delete;
        void expand(uint64_t new_size) {
            if ((capacity_ >= BYTEBUFFER_UINT_SIZE) && new_size < BYTEBUFFER_UINT_SIZE) { // assign algorithm
                new_size = BYTEBUFFER_UINT_SIZE;
            }
            char *new_ptr = new char[new_size];
            memset(new_ptr, 0, new_size);
            memcpy(new_ptr, data_, capacity_);
            delete[] data_;
            data_ = new_ptr;
            capacity_ = new_size;
        }
        void write(const char *buffer, uint64_t size, uint64_t offset) {
            if (offset + size > capacity_) {
                expand(offset + size);
            }
            memcpy(data_ + offset, buffer, size);
            write_callback_(size, offset);
        }
        void write(const char *buffer, uint64_t size) {
            if (limit_ + size > capacity_) {
                expand(limit_ + size);
            }
            memcpy(data_ + limit_, buffer, size);
            write_callback_(size, limit_);
        }

        template<typename T>
        void write(T t) {
            uint64_t size = sizeof(T);
            if (size + limit_ > capacity_) {
                expand(size + limit_);
            }
            if (!write_order_ && (limit_ - size < 0)) {
                throw bytebuffer_exception("error in write bytebuffer: EOF");
            }
            if (write_order_) {
                memcpy(data_ + limit_, &t, size);
                limit_ += size;
                write_callback_(size, limit_);
            } else {
                memcpy(data_ + limit_ - size, &t, size);
                write_callback_(size, limit_);
            }
        }
        void write(bytebuffer &&buffer) {
            if (buffer.limit_ + limit_ > capacity_) {
                expand(buffer.limit_ + limit_);
            }
            if (!write_order_ && (limit_ - buffer.limit_ < 0)) {
                throw bytebuffer_exception("error in write bytebuffer: EOF");
            }
            if (write_order_) {
                memcpy(data_ + limit_, buffer.data_, buffer.limit_);
                limit_ += buffer.limit_;
                write_callback_(buffer.limit_, limit_);
            } else {
                memcpy(data_ + limit_ - buffer.limit_, buffer.data_, buffer.limit_);
                write_callback_(buffer.limit_, limit_);
            }
        }
        char *read(uint64_t size, uint64_t offset) {
            read_callback_(size, offset);
            if (size + offset > capacity_) {
                throw bytebuffer_exception("error in read bytebuffer: EOF");
            }
            char *ptr = new char[size];
            memcpy(ptr, data_ + offset, size);
            return ptr;
        }

        template<typename T>
        T read() {
            uint64_t size = sizeof(T);
            read_callback_(size, position_);
            if ((read_order_ && (size + position_) > capacity_)) {
                if (position_ == capacity_) {
                    throw bytebuffer_exception("error in read bytebuffer: EOF");
                } else {
                    size = capacity_ - position_;
                }
            }
            if (!read_order_ && (position_ - size) < 0) {
                if (position_ == 0) {
                    throw bytebuffer_exception("error in read bytebuffer: EOF");
                } else {
                    size = position_;
                }
            }
            if (read_order_) {
                position_ += size;
                T t;
                memcpy(&t, data_ + position_ - size, size);
                return t;
            } else {
                position_ -= size;
                T t;
                memcpy(&t, data_ + position_, size);
                return t;
            }
        }
        bytebuffer read(uint64_t size, bool copy = true) {
            read_callback_(size, position_);
            if ((read_order_ && (size + position_) > capacity_)) {
                if (position_ == capacity_) {
                    throw bytebuffer_exception("error in read bytebuffer: EOF");
                } else {
                    size = capacity_ - position_;
                }
            }
            if (!read_order_ && (position_ - size) < 0) {
                if (position_ == 0) {
                    throw bytebuffer_exception("error in read bytebuffer: EOF");
                } else {
                    size = position_;
                }
            }
            if (read_order_) {
                position_ += size;
                return {data_ + position_ - size, size, copy};
            } else {
                position_ -= size;
                return {data_ + position_, size, copy};
            }
        }

        template<typename T>
        static bytebuffer make(T &&t) {
            bytebuffer buffer;
            auto size = sizeof(T);
            buffer.data_ = reinterpret_cast<char *>(&t);
            buffer.limit_ = size;
            buffer.capacity_ = size;
            buffer.can_free_ptr_ = false;
            return buffer;
        }
        template<typename T>
        static bytebuffer make(T &t) {
            bytebuffer buffer;
            auto size = sizeof(T);
            buffer.data_ = new char[size];
            memset(buffer.data_, 0, size);
            memcpy(buffer.data_, &t, size);
            buffer.limit_ = size;
            buffer.capacity_ = size;
            return buffer;
        }
        template<typename T> requires std::is_pointer_v<T>
        static bytebuffer make(T t, unsigned int len) {
            bytebuffer buffer;
            buffer.data_ = new char[len];
            memset(buffer.data_, 0, len);
            memcpy(buffer.data_, t, len);
            buffer.limit_ = len;
            buffer.capacity_ = len;
            return buffer;
        }
        template<typename T>
        T get() {
            assert(sizeof(T) == limit_);
            T t;
            memcpy(&t, data_, limit_);
            return t;
        }

        char &operator [](uint64_t pos) {
            return data_[pos];
        }

        uint64_t position() const {return position_;}
        void position(uint64_t v) {position_ = v;}
        uint64_t limit() const {return limit_;}
        void limit(uint64_t v) {limit_ = v;}
        uint64_t capacity() const {return capacity_;}
        void capacity(uint64_t v) {capacity_ = v;}
        void flip() {position_ = limit_;} // call after finally write done
        void order(bool r, bool w, bool b) {read_order_ = r; write_order_ = w; byte_order_ = b;}
        std::tuple<bool, bool, bool> order() {return std::make_tuple(read_order_, write_order_, byte_order_);}
        void reset() {
            if (can_free_ptr_) {
                delete[] data_;
            }
            capacity_ = 0;
            limit_ = 0;
            position_ = 0;
            can_free_ptr_ = true;
        }

        void write_callback(callback_t function) {
            write_callback_ = std::move(function);
        }
        void read_callback(callback_t function) {
            read_callback_ = std::move(function);
        }

        operator std::string () {
            return {data_, limit_};
        }
    };
}