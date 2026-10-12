
#include <cstring>

#include <algorithm>

#include <stdexcept>

#include <netinet/in.h>

#include "bigdata/natural.hh"

namespace bigdata {

Natural::Mpz::Mpz(bitscnt_t bits) {
    mpz_init2(n_, bits);
}

Natural::Mpz::~Mpz() {
    mpz_clear(n_);
}

Natural::Seed::Seed() {
    gmp_randinit_mt(randstate_);
}

Natural::Seed::~Seed() {
    gmp_randclear(randstate_);
}

Natural::Natural() :
    bits_(0) {
}

Natural::Natural(bitscnt_t bits, bool alloc) :
    bits_(bits) {
    if (alloc) {
        value_.reset(new Mpz(bits));
    }
}

Natural::Natural(uint32_t value) {
    bits_ = 1;
    value_.reset(new Mpz(bits_));
    mpz_set_ui(value_->n_, value);
    set_bits();
}

Natural::Natural(const Natural & natural) :
    bits_(natural.bits_) {
    if (bits_ > 0) {
        value_.reset(new Mpz(bits_));
        mpz_set (value_->n_, natural.value_->n_);
    }
}

Natural::Natural(Natural && natural) :
    bits_(natural.bits_) {
    value_.swap(natural.value_);
    natural.value_.reset();
    natural.bits_ = 0;
}

Natural Natural::high(bitscnt_t bits) const {
    if (bits <= 0 || value_ == NULL) return {};
    Natural natural(1, true);
    mpz_fdiv_q_2exp(natural.value_->n_, value_->n_, bits);
    natural.set_bits();
    return natural;
}


Natural Natural::low(bitscnt_t bits) const {
    if (bits <= 0 || value_ == NULL) return {};
    Natural natural(1, true);
    mpz_fdiv_r_2exp(natural.value_->n_, value_->n_, bits);
    natural.set_bits();
    return natural;
}

Natural Natural::random(Seed & seed, bitscnt_t bits) {
    Natural natural(1, true);
    mpz_urandomb (natural.value_->n_, seed.randstate_, bits);
    natural.set_bits();
    return natural;
}

int Natural::compare(const Natural & natural) const {
    if (bits_ == 0) {
        if (natural.bits_ == 0) return 0;
        else {
            if (natural != 0) return -1;
            else return 0;
        }
    } else {
        if (natural.bits_ == 0) {
            if (*this != 0) return 1;
            else return 0;
        }
    }
    if (value_ == NULL) {
        if (natural.value_ == NULL)
            return 0;
        else
            return -1;
    } else if (natural.value_ == NULL) {
        return 1;
    }
    return mpz_cmp(value_->n_, natural.value_->n_);
}

Natural & Natural::operator=(const Natural & natural) {
    if (this == &natural) return *this;
    bits_ = natural.bits_;
    if (natural.value_ == NULL) {
        value_.reset();
    } else {
        if (value_ == NULL) {
            value_.reset(new Mpz(bits_));
        }
        mpz_set (value_->n_, natural.value_->n_);
    }
    return *this;
}

Natural & Natural::operator=(uint32_t value) {
    bits_ = 1;
    if (value_ == NULL) {
        value_.reset(new Mpz(bits_));
    }
    mpz_set_ui (value_->n_, value);
    set_bits();
    return *this;
}

Natural::operator uint32_t() const {
    if (bits_ == 0) {
        return 0u;
    }
    if (value_) {
        if (mpz_fits_ulong_p(value_->n_)) {
            uint32_t value = static_cast<uint32_t>(mpz_get_ui(value_->n_));
            return value;
        } else {
            throw std::overflow_error("This natural doesn't fit into a uint32_t");
        }
    }
    return 0;

}

bool Natural::operator<(const Natural & natural) const {
    int res = compare(natural);
    return (res < 0);
}

bool Natural::operator<=(const Natural & natural) const {
    int res = compare(natural);
    return (res <= 0);  // not greater than
}

bool Natural::operator>(const Natural & natural) const {
    int res = compare(natural);
    return (res > 0);

}

bool Natural::operator>=(const Natural & natural) const {
    int res = compare(natural);
    return (res >= 0);  // not less than
}

bool Natural::operator==(const Natural & natural) const {
    int res = compare(natural);
    return (res == 0);
}

bool Natural::operator!=(const Natural & natural) const {
    int res = compare(natural);
    return (res != 0);
}

bool Natural::operator==(uint32_t value) const {
    if (bits_ == 0) {
        return false;  // return value == 0;
    }
    if (value_ != NULL) {
        if (mpz_fits_ulong_p(value_->n_)) {
            return mpz_get_ui(value_->n_) == value;
        }
    } else {
        return value == 0;
    }
    return false;
}

bool Natural::operator!=(uint32_t value) const {
    return !(*this == value);
}

size_t Natural::write(FILE *stream) const {
    size_t s = 0;
    size_t ret = 0;

    ret = fwrite(&bits_, sizeof(bits_), 1, stream);
    if (ret != 1) {
        throw std::runtime_error("Can't write Natural::id_");
    }
    s += sizeof(bits_);
    if (bits_ > 0) {
        if (value_ == NULL) {
            mpz_t aux;
            mpz_init(aux);
            ret = mpz_out_raw(stream, aux);
            mpz_clear(aux);
        } else {
            ret = mpz_out_raw(stream, value_->n_);
        }
        if (ret <= 0) {
            throw std::runtime_error("Can't write Natural::value_");
        }
        s += ret;
    }
    return s;
}

size_t Natural::read(FILE *stream) {
    size_t s = 0;
    size_t ret;

    // mpz_import

    ret = fread(&bits_, sizeof(bits_), 1, stream);
    if (ret != 1) {
        bits_ = 0;
        value_.reset();
        throw std::runtime_error("Can't read Natural::id_");
    }
    s += sizeof(bits_);

    if (bits_ > 0) {
        if (value_ == NULL) {
            value_.reset(new Mpz(bits_));
        }
        ret = mpz_inp_raw(value_->n_, stream);
        if (ret <= 0) {
            bits_ = 0;
            value_.reset();
            throw std::runtime_error("Can't read Natural::value_");
        }
        s += ret;
        set_bits();
    } else {
        value_.reset();
    }

    return s;
}

Natural Natural::operator<<(bitscnt_t shift) const {
    Natural natural(1, false);
    if (value_) {
        natural.value_ = std::make_unique<Mpz>(bits());
        mpz_mul_2exp (natural.value_->n_, value_->n_, shift);
        natural.set_bits();
    }

    return natural;
}

Natural Natural::operator>>(bitscnt_t shift) const {
    Natural natural(1, false);
    if (value_) {
        natural.value_ = std::make_unique<Mpz>(bits());
        mpz_tdiv_q_2exp (natural.value_->n_, value_->n_, shift);
        natural.set_bits();
    }

    return natural;
}

Natural & Natural::operator<<=(bitscnt_t shift) {
    Natural natural = (*this) << shift;
    *this = natural;
    return *this;
}

Natural & Natural::operator>>=(bitscnt_t shift) {
    Natural natural = (*this) >> shift;
    *this = natural;
    return *this;
}


Natural Natural::operator|(const Natural & natural) const {
    Natural new_natural(bits() > natural.bits() ? bits() : natural.bits(), false);
    if (value_) {
        new_natural.value_ = std::make_unique<Mpz>(new_natural.bits());
        if (natural.value_) {
            mpz_ior (new_natural.value_->n_, value_->n_, natural.value_->n_);
        } else {
            mpz_set (new_natural.value_->n_, value_->n_);
        }
        new_natural.set_bits();
    } else {
        if (natural.value_) {
            new_natural.value_ = std::make_unique<Mpz>(new_natural.bits());
            mpz_set (new_natural.value_->n_, natural.value_->n_);
            new_natural.set_bits();
        }
    }

    return new_natural;
}


void Natural::set_bits() {
    //mpz_sizeinbase (const mpz_t op, int base)
    if (value_->n_ == NULL) return;

    size_t size = mpz_size(value_->n_);

    if (bits_ == 0) {
        bits_ = 1;
    }

    if (size == 0) {
        return;
    }

    if (size == 1) {
        while((1ull << bits_) <= mpz_getlimbn(value_->n_, 0)) {
            bits_ <<= 1;
            if (bits_ == 0) {
                bits_ = sizeof(mp_limb_t) * 8;
                break;
            }
        }
    } else {
        while (bits_ < size * sizeof(mp_limb_t) * 8) {
            bits_ <<= 1;
            if (bits_ == 0) {
                throw std::runtime_error("This number doens't fit into 2^" + std::to_string(sizeof(bitscnt_t)*8) + " bits."
                                         + " alloc= " + std::to_string(value_->n_[0]._mp_alloc)
                        + ", size= " + std::to_string(value_->n_[0]._mp_size) + ".");
            }
        }
    }

}

std::string Natural::to_string() const {
    std::ostringstream stream;

    if (is_valid()) {
        if (value_ != NULL) {
            auto flags = stream.flags();
            stream << std::dec << bits_ << '_';
            if (bits_ <= sizeof(mp_limb_t) * 8) {
                stream << std::hex << value_->n_ << '_';
            } else {
                stream << std::hex << value_->n_[0]._mp_d[value_->n_[0]._mp_size-1] << '_';
                stream << std::hex << value_->n_[0]._mp_d[0];

            }
            stream.flags(flags);
        } else {
            stream << "natural_null";
        }
    } else {
        stream << "natural_invalid";
    }

    return stream.str();
}

std::ostream& operator << (std::ostream& stream, const Natural& natural) {
    stream << natural.to_string();
    return stream;
}

std::istream& operator >> (std::istream& stream, Natural& natural) {
    if (natural.value_ == NULL) {
        natural.value_ = std::make_unique<Natural::Mpz>(0);
    }
    stream >> natural.value_->n_;
    natural.set_bits();
    return stream;
}

}

