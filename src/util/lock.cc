#include "lock.hh"
#include <fcntl.h>
#include <stdexcept>
#include <sys/file.h>
#include <unistd.h>

ExclusiveLock::ExclusiveLock(const std::string &path) : lockPath(path), fd(-1) {
    fd = open(lockPath.c_str(), O_CREAT | O_RDWR, 0666);
    if (fd == -1) {
        locked_ = false;
        return;
    }

    if (flock(fd, LOCK_EX | LOCK_NB) == -1) {
        close(fd);
        locked_ = false;
        return;
    }
    locked_ = true;
}

bool ExclusiveLock::locked() { return locked_; }

ExclusiveLock::~ExclusiveLock() {
    if (fd != -1) {
        flock(fd, LOCK_UN);
        close(fd);
    }
}
