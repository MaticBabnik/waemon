#include "Socket.hh"

#include "util/log.hh"
#include <cstring>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

Socket::Socket(const std::string &socketPath) : socketPath(socketPath) {
    setupSocket();
}

Socket::~Socket() {
    close(epollFd);
    close(listenSock);
    unlink(socketPath.c_str());
}

void Socket::dispatch(std::function<void(std::string &, int)> messageHandler) {
    struct epoll_event events[IPC_N_CONN];

    int num_events = epoll_wait(epollFd, events, IPC_N_CONN, 0);

    for (int i = 0; i < num_events; i++) {
        if (events[i].data.fd == listenSock) handleNewConnection();
        else handleClientData(events[i].data.fd, messageHandler);
    }
}

void Socket::setupSocket() {
    listenSock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (listenSock < 0) {
        throw std::runtime_error("Couldn't start the socket");
    }

    struct sockaddr_un address = {.sun_family = AF_UNIX};
    strcpy(address.sun_path, socketPath.c_str());
    unlink(socketPath.c_str());

    if (bind(listenSock, (sockaddr *)&address, SUN_LEN(&address)) < 0) {
        close(listenSock);
        throw std::runtime_error("Couldn't bind the socket");
    }

    if (listen(listenSock, IPC_N_CONN) < 0) {
        close(listenSock);
        throw std::runtime_error("Couldn't listen on the socket");
    }

    setNonBlocking(listenSock);

    epollFd = epoll_create1(0);
    if (epollFd < 0) {
        close(listenSock);
        throw std::runtime_error("Couldn't create epoll instance");
    }

    epoll_event event;
    event.events  = EPOLLIN;
    event.data.fd = listenSock;

    if (epoll_ctl(epollFd, EPOLL_CTL_ADD, listenSock, &event) < 0) {
        close(listenSock);
        close(epollFd);
        throw std::runtime_error("Couldn't add socket to epoll");
    }

    logger::info("Bound to socket {}", socketPath);
}

void Socket::setNonBlocking(int sock) {
    int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);
}

void Socket::handleNewConnection() {
    sockaddr_in clientAddress = {};
    socklen_t   clientSize    = sizeof(clientAddress);
    int conn = accept(listenSock, (sockaddr *)&clientAddress, &clientSize);

    if (conn < 0) {
        logger::warn("Couldn't accept connection");
        return;
    }

    setNonBlocking(conn);
    epoll_event event;
    event.events  = EPOLLIN | EPOLLET;
    event.data.fd = conn;

    if (epoll_ctl(epollFd, EPOLL_CTL_ADD, conn, &event) < 0) {
        logger::warn("Couldn't add connection {} to epoll", conn);
        close(conn);
    }
}

void Socket::handleClientData(
    int                                      conn,
    std::function<void(std::string &, int)> &messageHandler
) {
    char readBuffer[16 * 1024] = {0};
    int  messageSize           = read(conn, readBuffer, sizeof(readBuffer) - 1);

    if (messageSize <= 0) {
        close(conn);
        return;
    }

    readBuffer[messageSize] = '\0';
    std::string message(readBuffer);

    messageHandler(message, conn);
}