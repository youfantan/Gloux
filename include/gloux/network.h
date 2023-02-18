#pragma once
#include <arpa/inet.h>
#include <string>
#include <list>

namespace gloux::network {
    class InetAddress {
    private:
        struct sockaddr_in addr{};
        socklen_t addr_len{};
    public:
        InetAddress();
        InetAddress(const std::string& ip, uint16_t port);
        InetAddress(sockaddr_in addr, socklen_t addr_len);

        sockaddr_in& get_sockaddr();
        socklen_t& getsocklen();
        const char* get_ip() const;
        uint16_t get_port() const;
    };

    class Socket {
    private:
        int fd;
        bool closed;
        bool bind_mode;
        InetAddress address;
    public:
        Socket();
        explicit Socket(int fd);
        Socket(int fd, InetAddress& address);

        void bind(InetAddress& address);
        void connect(InetAddress& address);
        void listen() const;
        Socket& accept() const;
        void set_nonblocking() const;
        int get_fd() const;
        InetAddress& get_address();
        bool is_closed() const;
        bool is_bind_mode() const;
        void close();
        void free();
    };
}