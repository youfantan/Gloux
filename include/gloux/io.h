#pragma once

#include <cstdint>
#include <memory>
#include <cstring>

constexpr int DEFAULT_AUTO_RESIZE_LENGTH = 128 * 1024;

namespace gloux::io {
    class ByteBuffer {
    private:
        char *ptr;
        uint64_t position;
        uint64_t capacity;
        bool little_endian;
        bool positive_order;

        template<class T, typename = typename std::enable_if_t<std::is_integral_v<T>>>
        T little_endian_to_big_endian(T data) {
            if (sizeof(T) == 2) {
                return __builtin_bswap16((uint16_t)data);
            } else if (sizeof(T) == 4) {
                return __builtin_bswap32((uint32_t)data);
            } else if (sizeof(T) == 8) {
                return __builtin_bswap64((uint64_t)data);
            }
        }
    public:
        explicit ByteBuffer(uint64_t size);
        explicit ByteBuffer(char *ptr, uint64_t length);
        ~ByteBuffer();
        void resize(uint64_t length);
        void auto_resize();
        std::unique_ptr<char*> read_copy(uint64_t offset, uint64_t length);
        std::unique_ptr<char*> get(uint64_t length);
        void write(uint64_t offset, void *ptr, uint64_t length);
        void append(void *ptr, uint64_t length);

        template<class T, typename = typename std::enable_if_t<std::is_integral_v<T>>>
        T read_ref_data() {
            char *data_ptr = ptr + position;
            return *reinterpret_cast<T*>(data_ptr);
        }
        template<class T, typename = typename std::enable_if_t<std::is_integral_v<T>>>
        T read_copy_data() {
            int data_size = sizeof(T);
            if (position - data_size < 0) {
                return 0;
            }
            T *data = new T;
            char *data_ptr = nullptr;
            if (positive_order) {
                 data_ptr = ptr + position;
                 position += data_size;
            } else {
                data_ptr = ptr + position - data_size;
                position -= data_size;
            }
            memcpy(data, data_ptr, data_size);
            if (!little_endian) {
                return little_endian_to_big_endian(*data);
            }
            return *data;
        }
        template<class T, typename = typename std::enable_if_t<std::is_integral_v<T>>>
        void write_data(T data) {
            int data_size = sizeof(T);
            if (position + data_size >= capacity) {
                auto_resize();
            }
            data = little_endian_to_big_endian(data);
            char* data_ptr = nullptr;
            if (positive_order) {
                data_ptr = ptr + position;
                position += data_size;
            } else {
                data_ptr = ptr + position - data_size;
                position -= data_size;
            }
            memcpy(data_ptr, &data, data_size);
        }

        void set_read_order(bool positive);
        void set_byteorder(bool little);
    };
}