/*
 * Util.hh
 *
 *  Created on: Jul 8, 2015
 *      Author: edgard
 */

#ifndef BIGDATA_UTIL_HH_
#define BIGDATA_UTIL_HH_

#include <cstdint>
#include <iomanip>
#include <locale>
#include <sstream>


#define LIKELY(x)    __builtin_expect (!!(x), 1)
#define UNLIKELY(x)  __builtin_expect (!!(x), 0)

#define POW2(x) (((uint64_t)2)<<((x)-1))

namespace bigdata {

extern uint64_t get_proc_memory();

extern uint64_t get_total_memory();
extern uint64_t get_available_memory();

extern uint32_t log2(uint32_t value);

/*
template<class T>
std::string numberFormatWithCommas(T value){
    struct Numpunct: public std::numpunct<char>{
    protected:
        virtual char do_thousands_sep() const{return ',';}
        virtual std::string do_grouping() const{return "\03";}
    };
    std::stringstream ss;
    ss.imbue({std::locale(), new Numpunct});
    ss << std::setprecision(2) << std::fixed << value;
    return ss.str();
}
*/

template<class T>
std::string format_with_commas(T value, int32_t precision=-1)
{
    std::stringstream ss;
    ss.imbue(std::locale(""));
    if (precision < 0)
        ss << std::fixed << value;
    else
        ss << std::setprecision(precision) << std::fixed << value;
    return ss.str();
}

// extern std::pair<bool, size_t> read_all(std::shared_ptr<boost::asio::ip::tcp::socket> sock, char * data, size_t max_size);

extern std::string Hex(const void *buffer, unsigned size, const char * line_indent);

}

#endif  // BIGDATA_UTIL_HH_
