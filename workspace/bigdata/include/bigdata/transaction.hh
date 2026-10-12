

#ifndef BIGDATA_TRANSACTION_HH_
#define BIGDATA_TRANSACTION_HH_

#include <sys/types.h>
#include <unistd.h>

#include <cstdint>

#include <mutex>
#include <unordered_map>
#include <set>
#include <array>

#include "bigdata/iserializable.hh"

namespace bigdata {

class TransactionSet;

class Transaction : public ISerializable {
public:
    Transaction();
    explicit Transaction(const Transaction & transaction);
    //explicit Transaction(Transaction && transaction);
    explicit Transaction(const uint8_t * mac);
    virtual ~Transaction() = default;

    bool operator==(const Transaction & transaction) const;
    bool operator!=(const Transaction & transaction) const;

    void set_mac(const uint8_t * mac);
    void renew();

    virtual size_t write(FILE *stream) const;
    virtual size_t read(FILE *stream);

    bool am_i_owner(const uint8_t * mac) const;

private:
    static uint32_t unique_transaction_;
    static std::mutex unique_mutex;

    uint32_t get_unique() const;

    uint32_t id_;
    pid_t pid_;
    std::array<uint8_t, 6> mac_;

    friend std::ostream& operator <<(std::ostream& stream, const Transaction& transaction);
    friend class TransactionSet;
};

class TransactionSet {
public:
    void insert(const Transaction & t);
    bool has(const Transaction & t) const;
    void remove(const Transaction & t);

    std::recursive_mutex & mutex() { return mutex_; }

private:
    mutable std::recursive_mutex mutex_;
    std::unordered_map<uint32_t, std::unordered_map<pid_t, std::set<std::array<uint8_t, 6> >   > > transactions_;
};

extern std::ostream& operator <<(std::ostream& stream, const Transaction& transaction);

}


#endif  // BIGDATA_TRANSACTION_HH_
