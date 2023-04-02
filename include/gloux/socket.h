#pragma once

#include <netinet/in.h>
#include <string>
#include <arpa/inet.h>
#include <cstring>
#include <utility>
#include <unistd.h>
#include "utils.h"
#include "io.h"
#include <sys/uio.h>

namespace gloux::network {
#define THROWIF(expr, msg) if ((expr) == -1) throw socket_exception(msg)

    class socket_exception : public std::exception {
    private:
        const std::string msg_;
    public:
        explicit socket_exception(std::string msg) : msg_(std::move(msg)) {}
        const char* what() {
            return msg_.c_str();
        }
    };

    class inet_address {
    public:
        inet_address() = default;
        inet_address(const inet_address &addr) {
            host_ = addr.host_;
            port_ = addr.port_;
            memcpy(&addr_, &addr.addr_, addr.socklen_);
        }
        inet_address(const inet_address &&addr) noexcept {
            host_ = addr.host_;
            port_ = addr.port_;
            memcpy(&addr_, &addr.addr_, addr.socklen_);
        }
        inet_address & operator = (const inet_address &addr) {
            host_ = addr.host_;
            port_ = addr.port_;
            memcpy(&addr_, &addr.addr_, addr.socklen_);
            return *this;
        }
        inet_address & operator = (inet_address &&addr) noexcept {
            host_ = addr.host_;
            port_ = addr.port_;
            memcpy(&addr_, &addr.addr_, addr.socklen_);
            return *this;
        }
        inet_address(const std::string &host, const uint16_t port, bool ipv4 = true) : host_(host), port_(port), ipv4_(ipv4) {
            if (ipv4) {
                addr_.sin_family = AF_INET;
                inet_pton(AF_INET, host.c_str(), &addr_.sin_addr);
            } else {
                addr_.sin_family = AF_INET6;
                inet_pton(AF_INET6, host.c_str(), &addr_.sin_addr);
            }
            addr_.sin_port = htons(port);
        }
        inet_address(const sockaddr_in &addr, const socklen_t &socklen, bool ipv4 = true) :  ipv4_(ipv4) {
            char host[128] = {0};
            if (ipv4) {
                inet_ntop(AF_INET, &addr.sin_addr, host, socklen);
            } else {
                inet_ntop(AF_INET6, &addr.sin_addr, host, socklen);
            }
            host_ = std::string(host);
            port_ = ntohs(addr.sin_port);
            memcpy(&addr_, &addr, socklen);
        }
        inet_address(const sockaddr_in *addr, const socklen_t *socklen, bool ipv4 = true) : ipv4_(ipv4) {
            char host[128] = {0};
            if (ipv4) {
                inet_ntop(AF_INET, &addr->sin_addr, host, *socklen);
            } else {
                inet_ntop(AF_INET6, &addr->sin_addr, host, *socklen);
            }
            host_ = std::string(host);
            port_ = ntohs(addr->sin_port);
            memcpy(&addr_, addr, *socklen);
        }
        void trans() {
            char host[128] = {0};
            if (ipv4_) {
                inet_ntop(AF_INET, &addr_.sin_addr, host, socklen_);
            } else {
                inet_ntop(AF_INET6, &addr_.sin_addr, host, socklen_);
            }

            host_ = std::string(host);
            port_ = ntohs(addr_.sin_port);
        }

        static bool construct_by_fd(int &fd, inet_address &addr) {
            THROWIF(getsockname(fd, reinterpret_cast<sockaddr *>(&addr.addr_), &addr.socklen_), "error in get socket name");
            addr.trans();
            return true;
        }

        uint16_t port_{};
        sockaddr_in addr_{};
        socklen_t socklen_ {sizeof(addr_)};
        std::string host_;
        bool ipv4_{true};
    };

    class tcp_socket {
    public:
        inline void init_callback() {
            w_buffer.write_callback([&](auto size, auto off) {
                this->write_buffer(this->w_buffer.read(size, false));
            });
            r_buffer.read_callback([&](auto size, auto off) {
                this->read_buffer(size);
            });
        }
        tcp_socket() {
            init_callback();
        } // constructor only fill data_ structure
        tcp_socket(int &sockfd, inet_address &addr);
        tcp_socket(int &sockfd, inet_address &&addr);
        bool create(); // just create socket file descriptor
        bool bind(inet_address &address); // bind socket to a specific IP and port
        bool bind(inet_address &&address); // pass right value(in situ construct)
        bool bind() const; // seem to be useless
        bool connect(inet_address &address); // SYN, ACK ...
        bool connect(inet_address &&address);
        bool connect();
        bool listen() const; // enter status of ESTABLISHED
        tcp_socket accept() const; // receive SYN and ACK
        void shutdown() const; // send FIN
        void close() const; // send FIN and close file descriptor
        io::bytebuffer &read_buffer(unsigned int buffer_size);
        void write_buffer(io::bytebuffer &buffer);
        void write_buffer(io::bytebuffer &&buffer);

        int sockfd_{-1};
        inet_address addr_{};
        io::bytebuffer r_buffer{};
        io::bytebuffer w_buffer{};
    };

    tcp_socket::tcp_socket(int &sockfd, inet_address &addr) : sockfd_(sockfd), addr_(inet_address(addr)) {
        init_callback();
    }

    tcp_socket::tcp_socket(int &sockfd, inet_address &&addr) : sockfd_(sockfd) , addr_(std::forward<inet_address>(addr)) {
        init_callback();
    }

    bool tcp_socket::create() {
        if (addr_.host_.empty()) {
            THROWIF(this->sockfd_ = ::socket(AF_INET, SOCK_STREAM, 0), "error in create socket");
        }
        return true;
    }

    bool tcp_socket::bind(inet_address &address) {
        addr_ = address;
        THROWIF(::bind(sockfd_, reinterpret_cast<const sockaddr *>(&address.addr_), address.socklen_), "error in bind socket");
        return true;
    }

    bool tcp_socket::bind(inet_address &&address) {
        addr_ = std::forward<inet_address>(address);
        THROWIF(::bind(sockfd_, reinterpret_cast<const sockaddr *>(&address.addr_), address.socklen_), "error in bind socket");
        return true;
    }

    bool tcp_socket::bind() const {
        if (addr_.host_.empty()) {
            throw socket_exception("error in bind: host is empty");
        }
        THROWIF(::bind(sockfd_, reinterpret_cast<const sockaddr *>(&addr_.addr_), addr_.socklen_), "error in bind socket");
        return true;
    }

    bool tcp_socket::connect(inet_address &address) {
        THROWIF(::connect(sockfd_, reinterpret_cast<const sockaddr *>(&address.addr_), address.socklen_), "error in connect socket");
        inet_address::construct_by_fd(sockfd_, addr_);
        return true;
    }

    bool tcp_socket::connect(inet_address &&address) {
        THROWIF(::connect(sockfd_, reinterpret_cast<const sockaddr *>(&address.addr_), address.socklen_), "error in connect socket");
        inet_address::construct_by_fd(sockfd_, addr_);
        return true;
    }

    bool tcp_socket::connect() {
        if (addr_.host_.empty()) {
            throw socket_exception("error in connect: host is empty");
        }
        THROWIF(::connect(sockfd_, reinterpret_cast<const sockaddr *>(&addr_.addr_), addr_.socklen_), "error in connect socket");
        return true;
    }

    bool tcp_socket::listen() const {
        THROWIF(::listen(sockfd_, SOMAXCONN), "error in listen socket");
        return true;
    }

    tcp_socket tcp_socket::accept() const {
        inet_address cli_addr;
        int sockfd;
        THROWIF(sockfd = ::accept(sockfd_, reinterpret_cast<sockaddr *>(&cli_addr.addr_), &cli_addr.socklen_), "error in accept socket");
        cli_addr.trans();
        return {sockfd, std::move(cli_addr)};
    }

    void tcp_socket::shutdown() const {
        THROWIF(::shutdown(sockfd_, SHUT_RDWR), "error in shutdown socket");
    }

    void tcp_socket::close() const {
        ::close(sockfd_);
    }

    io::bytebuffer &tcp_socket::read_buffer(unsigned int buffer_size) {
        ssize_t bytes_read{};
        char *buf = new char[buffer_size];
        do {
            THROWIF(bytes_read = recv(sockfd_, buf, buffer_size, 0), "error in read socket buffer");
            r_buffer.write(buf, bytes_read);
        } while (bytes_read != 0);
        delete[] buf;
        return r_buffer;
    }

    void tcp_socket::write_buffer(io::bytebuffer &buffer) {
        THROWIF(send(sockfd_, &buffer[0], buffer.limit(), 0), "error in write socket buffer");
    }

    void tcp_socket::write_buffer(io::bytebuffer &&buffer) {
        THROWIF(send(sockfd_, &buffer[0], buffer.limit(), 0), "error in write socket buffer");
    }
}