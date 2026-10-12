
#include <cstdio>
#include <cstring>

#include <stdexcept>

#include "bigdata/id.hh"
#include "bigdata/util.hh"

namespace bigdata {

Id::Id() : y_(invalid_y) {
}

Id::Id(const Id & id) : y_(id.y_), id_(id.id_) {

}

Id::Id(FILE *stream, size_t * ret) : y_(invalid_y) {
    if (ret != NULL) {
        *ret = 0;
        *ret = read(stream);
    } else {
        read(stream);
    }
}

Id & Id::operator=(const Id & id) {
    y_ = id.y_;
    id_ = id.id_;
    return *this;
}


Id::Id(Id && id) : y_(id.y_), id_(id.id_) {
    id.y_ = invalid_y;
}

bool Id::operator==(const Id & id) const {
    return ((y_ == id.y_) && (id_ == id.id_));
}

bool Id::operator!=(const Id & id) const {
    return !(*this == id);
}

bool Id::operator<(const Id & id) const {
    if (y_ < id.y_) return true;
    if (y_ > id.y_) return false;

    if (id_ < id.id_) return true;
    if (id_ > id.id_) return false;

    return false;
}

bool Id::operator<=(const Id & id) const {
    if (y_ < id.y_) return true;
    if (y_ > id.y_) return false;

    if (id_ < id.id_) return true;
    if (id_ > id.id_) return false;

    return true;
}

bool Id::operator>(const Id & id) const {
    return !(*this <= id);
}

bool Id::operator>=(const Id & id) const {
    return !(*this < id);
}

std::ostream& operator <<(std::ostream& stream, const Id & id) {
    auto flags = stream.flags();
    stream << std::dec << (id.y_ >> 27) << '_' << std::hex << (id.y_ & 0x7FFFFFF) << '-' << id.id_;
    stream.flags(flags);
    return stream;
}

size_t Id::write(FILE *stream) const {
    size_t s = 0;
    size_t ret = 0;

    ret = fwrite(&y_, sizeof(y_), 1, stream);
    if (ret != 1) {
        throw std::runtime_error("Can't write Id::y_");
    }
    s += sizeof(y_);
    ret = id_.write(stream);
    s += ret;

    return s;
}

size_t Id::read(FILE *stream) {
    size_t s = 0;
    size_t ret = 0;

    ret = fread(&y_, sizeof(y_), 1, stream);
    if (ret != 1) {
        throw std::runtime_error(__FILE__ + std::to_string(__LINE__) + ":Can't read Id::y_");
    }
    s += sizeof(y_);
    s += id_.read(stream);
    return s;
}

bool Id::is_summary(const Id & id, bool check_parents) {
    uint32_t level = id.y_ >> 27;
    uint32_t smask = id.y_ & 0x7FFFFFF;

    if (check_parents) {
        return smask != 0;
    }

    return ((1u << (level - 1u)) & smask);
}

Id Id::calculate_id(bitscnt_t universe) {
    Id id;

    uint32_t level = bigdata::log2(universe) + 1;
    uint32_t smask = 0x00;

    id.y_ = (level << 27) | smask;

    return id;
}

Id Id::calculate_child_id(const Id & parent, bool summary, const Natural & index) {
    Id id;

    uint32_t parent_level = parent.y_ >> 27;
    uint32_t parent_smask = parent.y_ & 0x7FFFFFF;

    if (parent_level == 1) {
        throw std::runtime_error("Parent is already 2^1. You are trying to calculate the child's Id!");
    }

    uint32_t level = parent_level - 1;
    uint32_t smask = parent_smask;

    if (summary) {
        smask |= 1 << (level - 1);
        id.id_ = parent.id_;
    } else {
        id.id_ = parent.id_ | (index << (1u << (level - 1)));
    }
    id.y_ = (level << 27) | smask;


    return id;
}


}
