#include "gloux/io.h"

namespace gloux::io {
    ByteBuffer::ByteBuffer(uint64_t size) : position(0), capacity(size), positive_order(true), little_endian(true){
        this->ptr = new char[size];
        bzero(ptr, size);
    }

    ByteBuffer::ByteBuffer(char *ptr, uint64_t length) : position(0), capacity(length), positive_order(true), little_endian(true){
        this->ptr = new char[length];
        memcpy(this->ptr, ptr, length);
    }

    ByteBuffer::~ByteBuffer() {
        free(ptr);
    }

    void ByteBuffer::resize(uint64_t length) {
        char *new_ptr = new char[length];
        memcpy(new_ptr, ptr, capacity);
        free(ptr);
        this->ptr = new_ptr;
        this->capacity = length;
    }

    void ByteBuffer::auto_resize() {
        resize(capacity + DEFAULT_AUTO_RESIZE_LENGTH);
    }

    std::unique_ptr<char *> ByteBuffer::read_copy(uint64_t offset, uint64_t length) {
        std::unique_ptr<char*> ret_ptr = std::make_unique<char*>(new char[length]);
        char *data_ptr = ptr + offset;
        memcpy(ret_ptr.get(), data_ptr, length);
        return ret_ptr;
    }

    std::unique_ptr<char*> ByteBuffer::get(uint64_t length) {
        std::unique_ptr<char*> ret_ptr = std::make_unique<char*>(new char[length]);
        char *data_ptr = nullptr;
        if (positive_order) {
            data_ptr = ptr + position;
            position += length;
        } else {
            data_ptr = ptr + position - length;
            position -= length;
        }
        memcpy(ret_ptr.get(), data_ptr, length);
        return ret_ptr;
    }

    void ByteBuffer::write(uint64_t offset, void *ptr, uint64_t length) {
        if (position + length >= capacity) {
            auto_resize();
        }
        char *data_ptr = reinterpret_cast<char*>(ptr) + offset;
        memcpy(data_ptr, ptr, length);
    }

    void ByteBuffer::append(void *ptr, uint64_t length) {
        if (position + length >= capacity) {
            auto_resize();
        }
        char *data_ptr = nullptr;
        if (positive_order) {
            data_ptr = this->ptr + position;
            position += length;
        } else {
            data_ptr = this->ptr + position - length;
            position -= length;
        }
        memcpy(data_ptr, ptr, length);
    }

    void ByteBuffer::set_read_order(bool positive) {
        this->positive_order = positive;
    }

    void ByteBuffer::set_byteorder(bool little) {
        this->little_endian = little;
    }

}