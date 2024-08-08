#include <functional>
#include <string>

#define IPC_N_CONN 10

class Socket {
  public:
    Socket(const std::string &socketPath);
    ~Socket();

    void dispatch(std::function<void(std::string &, int)> messageHandler);

  private:
    std::string socketPath;

    int listenSock;
    int epollFd;

    void setupSocket();
    void setNonBlocking(int sock);
    void handleNewConnection();
    void handleClientData(
        int                                      conn,
        std::function<void(std::string &, int)> &messageHandler
    );
};