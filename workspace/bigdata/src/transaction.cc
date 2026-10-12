
#include "bigdata/transaction.hh"

#include <errno.h>

#include <cstdio>
#include <cstring>

#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <arpa/inet.h>

namespace bigdata {

uint32_t Transaction::unique_transaction_ = 0;
std::mutex Transaction::unique_mutex;

uint32_t Transaction::get_unique() const {
    std::lock_guard<std::mutex> lk(unique_mutex);
    return ++unique_transaction_;
}

Transaction::Transaction()
    : id_(get_unique())
    , pid_(getpid())
    , mac_{0,0,0,0,0,0}
{
}


Transaction::Transaction(const Transaction & transaction)
    : id_(transaction.id_)
    , pid_(transaction.pid_)
    , mac_(transaction.mac_)
{
}

//Transaction::Transaction(Transaction && transaction) {
    //std::swap(id_, transaction.id_);
    //std::swap(pid_, transaction.pid_);
    //std::swap(mac_, transaction.mac_);
//}


Transaction::Transaction(const uint8_t * mac)
    : id_(get_unique())
    , pid_(getpid())
    , mac_{0,0,0,0,0,0}
{
    if (mac != NULL) {
        memcpy(mac_.data(), mac, 6);
    }
}

void Transaction::set_mac(const uint8_t * mac) {
    if (mac != NULL && mac != mac_.data()) {
        memcpy(mac_.data(), mac, 6);
    }
}

void Transaction::renew() {
    id_ = get_unique();
}

bool Transaction::am_i_owner(const uint8_t * mac) const {
    if (pid_ == getpid()) {
        if (0 == memcmp(mac, mac_.data(), 6)) {
            return true;
        }
    }
    return false;
}

size_t Transaction::write(FILE *stream) const {
    size_t nbytes = 0;
    size_t ret = 0;

    auto id_net = htonl(id_);

    ret = fwrite(&id_net, sizeof(id_net), 1, stream);
    if (ret != 1) {
        throw std::runtime_error(__FILE__ + std::to_string(__LINE__) + ":Can't write transaction.id_. Reason: " + std::string(strerror(errno)));
    }
    nbytes += sizeof(id_net);

    ret = fwrite(&pid_, sizeof(pid_), 1, stream);
    if (ret != 1) {
        throw std::runtime_error(__FILE__ + std::to_string(__LINE__) + ":Can't write transaction.pid_. Reason: " + std::string(strerror(errno)));
    }
    nbytes += sizeof(pid_);

    ret = fwrite(mac_.data(), 6, 1, stream);
    if (ret != 1) {
        throw std::runtime_error(__FILE__ + std::to_string(__LINE__) + ":Can't write transaction.mac_. Reason: " + std::string(strerror(errno)));
    }
    nbytes += 6;

    return nbytes;
}

bool Transaction::operator==(const Transaction & transaction) const {
    return id_ == transaction.id_ && pid_ == transaction.pid_ && 0 == memcmp(mac_.data(), transaction.mac_.data(), 6);
}

bool Transaction::operator!=(const Transaction & transaction) const {
    return !(*this == transaction);
}

size_t Transaction::read(FILE *stream) {
    size_t nbytes = 0;
    size_t ret = 0;

    ret = fread(&id_, sizeof(id_), 1, stream);
    if (ret != 1) {
        throw std::runtime_error(__FILE__ + std::to_string(__LINE__) + ":Can't read transaction.id_. Reason: " + std::string(strerror(errno)));
    }
    nbytes += sizeof(id_);
    id_ = ntohl(id_);

    ret = fread(&pid_, sizeof(pid_), 1, stream);
    if (ret != 1) {
        throw std::runtime_error(__FILE__ + std::to_string(__LINE__) + ":Can't read transaction.pid_. Reason: " + std::string(strerror(errno)));
    }
    nbytes += sizeof(pid_);

    ret = fread(mac_.data(), 6, 1, stream);
    if (ret != 1) {
        throw std::runtime_error(__FILE__ + std::to_string(__LINE__) + ":Can't read transaction.mac_. Reason: " + std::string(strerror(errno)));
    }
    nbytes += 6;

    return nbytes;
}

std::ostream& operator <<(std::ostream& stream, const Transaction& transaction) {
    auto flags = stream.flags();
    stream << std::hex << transaction.id_ << '.';
    stream << std::hex << std::setw(2) << std::setfill('0')
           << static_cast<int>(transaction.mac_[0]) << ':'
           << static_cast<int>(transaction.mac_[1]) << ':'
           << static_cast<int>(transaction.mac_[2]) << ':'
           << static_cast<int>(transaction.mac_[3]) << ':'
           << static_cast<int>(transaction.mac_[4]) << ':'
           << static_cast<int>(transaction.mac_[5]) << ':';
    stream.flags(flags);
    return stream;
}

void TransactionSet::insert(const Transaction & t) {
    std::lock_guard<std::recursive_mutex> lg(mutex_);

    auto & id = transactions_[t.id_];

    auto & pid = id[t.pid_];

    if (pid.find(t.mac_) == pid.end()) {
        pid.insert(t.mac_);
    }
}

bool TransactionSet::has(const Transaction & t) const {
    std::lock_guard<std::recursive_mutex> lg(mutex_);

    const auto id = transactions_.find(t.id_);

    if (id == transactions_.end()) return false;

    const auto pid = id->second.find(t.pid_);

    if (pid == id->second.end()) return false;

    auto mac = pid->second.find(t.mac_);

    if (mac == pid->second.end()) {
        return false;
    }

    return true;
}

void TransactionSet::remove(const Transaction & t) {
    std::lock_guard<std::recursive_mutex> lg(mutex_);

    auto id = transactions_.find(t.id_);

    if (id == transactions_.end()) return;

    auto pid = id->second.find(t.pid_);

    if (pid == id->second.end()) return;

    auto mac = pid->second.find(t.mac_);

    if (mac == pid->second.end()) {
        return;
    }

    pid->second.erase(mac);

    if (pid->second.size() == 0) {
        id->second.erase(pid);
    }

    if (id->second.size() == 0) {
        transactions_.erase(id);
    }
}

}
