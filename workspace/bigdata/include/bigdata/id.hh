
#ifndef BIGDATA_ID_HH_
#define BIGDATA_ID_HH_

#include <cstdint>

#include <ostream>

#include <boost/functional/hash.hpp>

#include "bigdata/iserializable.hh"
#include "bigdata/natural.hh"

namespace bigdata {

class Id : public ISerializable {
public:
    Id();
    Id(FILE *stream, size_t * ret);
    Id(const Id & id);
    Id(Id && id);
    virtual ~Id() = default;

    virtual size_t write(FILE *stream) const;
    virtual size_t read(FILE *stream);

    const uint32_t invalid_y = 0ul;

    uint32_t y_;
    Natural id_;

    Id & operator=(const Id & id);
    bool operator==(const Id & id) const;
    bool operator!=(const Id & id) const;
    bool operator<(const Id & id) const;
    bool operator<=(const Id & id) const;
    bool operator>(const Id & id) const;
    bool operator>=(const Id & id) const;

    friend std::ostream& operator <<(std::ostream& stream, const Id& id);

    static bool is_summary(const Id & id, bool check_parents = true);
    static Id calculate_id(bitscnt_t universe);
    static Id calculate_child_id(const Id & parent, bool summary, const Natural & index);
};

extern std::ostream& operator <<(std::ostream& stream, const Id& id);

}

namespace std
{
template<>
struct hash<bigdata::Id>
{
    size_t operator()(bigdata::Id const& id) const
    {
        size_t hash = 0;
        std::size_t h1 = std::hash<uint64_t>{}(id.y_);
        std::size_t h2 = std::hash<bigdata::Natural>{}(id.id_);
        boost::hash_combine(hash, h1);
        boost::hash_combine(hash, h2);
        return hash; // h1 ^ (h2 << 1);
    }
};
}


#endif  // BIGDATA_ID_HH_
