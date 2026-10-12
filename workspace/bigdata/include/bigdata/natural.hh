#ifndef BIGDATA_NATURAL_HH_
#define BIGDATA_NATURAL_HH_

#include <cstdint>
#include <climits>

#include <iostream>
#include <memory>

#include <boost/functional/hash.hpp>

#include <gmp.h>
#include <gmpxx.h>

#include "bigdata/iserializable.hh"

namespace bigdata {

typedef uint32_t bitscnt_t;

const bitscnt_t invalid_bitscnt = 0;

class Natural : public ISerializable {
private:
	class Mpz {
	public:
		explicit Mpz(bitscnt_t bits);
		~Mpz();
		mpz_t n_;
	};
public:
	class Seed {
	public:
		Seed();
		~Seed();
	private:
		gmp_randstate_t randstate_;
		friend class Natural;
	};

public:
	Natural();
    explicit Natural(uint32_t value);
    Natural(const Natural & natural);
    Natural(Natural && natural);

    virtual size_t write(FILE *stream) const;
    virtual size_t read(FILE *stream);

    explicit operator uint32_t() const;

	Natural & operator=(const Natural & natural);
    Natural & operator=(uint32_t value);

	bool operator<(const Natural & natural) const;
	bool operator<=(const Natural & natural) const;
	bool operator>(const Natural & natural) const;
	bool operator>=(const Natural & natural) const;
	bool operator==(const Natural & natural) const;
    bool operator!=(const Natural & natural) const;

    bool operator==(uint32_t value) const;
    bool operator!=(uint32_t value) const;

    Natural operator<<(bitscnt_t shift) const;
    Natural operator>>(bitscnt_t shift) const;
    Natural & operator<<=(bitscnt_t shift);
    Natural & operator>>=(bitscnt_t shift);
    Natural operator|(const Natural & natural) const    ;

	int compare(const Natural & natural) const;

    Natural high(bitscnt_t bits) const;
    Natural low(bitscnt_t bits) const;

    static Natural random(Seed & seed, bitscnt_t bits);

    inline bool is_valid() const {
        return bits_ > 0;
    }

	inline bitscnt_t bits() const {
		return bits_;
	}

    std::string to_string() const;

private:
	bitscnt_t bits_;
	std::unique_ptr<Mpz> value_;

    void set_bits();

    explicit Natural(bitscnt_t bits, bool alloc);

    friend std::ostream& operator << (std::ostream& stream, const Natural& natural);
    friend std::istream& operator >> (std::istream& stream, Natural& natural);
    friend class std::hash<Natural>;

};

extern std::ostream& operator << (std::ostream& stream, const Natural& natural);
extern std::istream& operator >> (std::istream& stream, Natural& natural);

}

namespace std
{
    template<>
    struct hash<bigdata::Natural>
    {
        size_t operator()(bigdata::Natural const& natural) const
        {
            size_t hash = 0x00;
            if (natural.value_ != 0) {
                size_t count = mpz_size(natural.value_->n_);
                const mp_limb_t * limbs = mpz_limbs_read (natural.value_->n_);
                for (auto c=0u; c < count; c++) {
                    uint64_t limb = limbs[c];
                    boost::hash_combine(hash, limb & 0xFFFFFFFF);
                    boost::hash_combine(hash, (limb >> 32) & 0xFFFFFFFF);
                }
            }
            return hash;
        }
    };
}


#endif  // BIGDATA_NATURAL_HH_
