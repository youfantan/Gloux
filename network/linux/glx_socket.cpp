#include <cstring>
#include "gloux/log.h"
#include "gloux/network.h"
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>

namespace gloux::network {
    InetAddress::InetAddress(const std::string& ip, uint16_t port) {
        bzero(&addr, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr(ip.c_str());
        addr.sin_port = htons(port);
        addr_len = sizeof(addr);
    }

    InetAddress::InetAddress(sockaddr_in addr, socklen_t addr_len) : addr(addr), addr_len(addr_len) {}

    sockaddr_in& InetAddress::get_sockaddr() {
        return addr;
    }

    socklen_t& InetAddress::getsocklen() {
        return addr_len;
    }

    const char* InetAddress::get_ip() const {
        return inet_ntoa(addr.sin_addr);
    }

    uint16_t InetAddress::get_port() const {
        return htons(addr.sin_port);
    }

    InetAddress::InetAddress() {
        addr_len = sizeof(addr);
        bzero(&addr, addr_len);
    }

    Socket::Socket() : closed(false), bind_mode(false) {
        fd = socket(AF_INET, SOCK_STREAM,  0);
    }
    Socket::Socket(int fd) : fd(fd), closed(false), bind_mode(false) {}
    Socket::Socket(int fd, InetAddress &address) : fd(fd), address(address), closed(false), bind_mode(false) {}

    void Socket::bind(InetAddress &address) {
        this->address = address;
        ERRIF(::bind(fd, (sockaddr*)&address.get_sockaddr(), address.getsocklen()) == -1, "Error in Bind Socket");
        bind_mode = true;
    }

    void Socket::connect(InetAddress &address) {
        this->address = address;
        ERRIF(::connect(fd, (sockaddr*)&address.get_sockaddr(), address.getsocklen()) == -1, "Error in Connect Socket");
        bind_mode = false;
    }

    void Socket::listen() const {
        ERRIF(::listen(fd, SOMAXCONN) == -1, "Error in Listen Socket");
    }

    Socket &Socket::accept() const {
        auto *client_address = new InetAddress;
        int client_fd = ::accept(fd, (sockaddr*)&client_address->get_sockaddr(), &client_address->getsocklen());
        ERRIF(client_fd == -1, "Error in Accept Socket");
        auto *client = new Socket(client_fd, *client_address);
        return *client;
    }

    void Socket::set_nonblocking() const {
        ERRIF(fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK) == -1, "Error in Set Nonblocking Socket");
    }

    int Socket::get_fd() const {
        return fd;
    }

    InetAddress &Socket::get_address() {
        return address;
    }

    bool Socket::is_closed() const {
        return closed;
    }

    bool Socket::is_bind_mode() const {
        return bind_mode;
    }

    void Socket::close() {
        ::close(fd);
        closed = true;
    }

    void Socket::free() {
        delete &address;
        delete this;
    }
}