#include <string>

class ExclusiveLock {
  public:
    ExclusiveLock(const std::string &path);
    ~ExclusiveLock();

    // Prevent copying
    ExclusiveLock(const ExclusiveLock &)            = delete;
    ExclusiveLock &operator=(const ExclusiveLock &) = delete;

    bool locked();

  private:
    int         fd;
    std::string lockPath;
    bool        locked_;
};