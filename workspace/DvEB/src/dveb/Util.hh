/*
 * Util.hh
 *
 *  Created on: Jul 8, 2015
 *      Author: edgard
 */

#ifndef UTIL_HH_
#define UTIL_HH_

#include <cstdint>
#include <iomanip>
#include <locale>
#include <sstream>

#include <boost/asio.hpp>

#define POW2(x) (((uint64_t)2)<<((x)-1))

extern uint64_t getUsedMemory();

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
std::string FormatWithCommas(T value)
{
    std::stringstream ss;
    ss.imbue(std::locale(""));
    ss << std::fixed << value;
    return ss.str();
}

// extern std::pair<bool, size_t> read_all(std::shared_ptr<boost::asio::ip::tcp::socket> sock, char * data, size_t max_size);

extern std::string Hex(const void *buffer, unsigned size, const char * line_indent);

#endif /* UTIL_HH_ */
