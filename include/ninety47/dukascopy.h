#ifndef _ninety47_libbi5_header_109823as0920fs8_
#define _ninety47_libbi5_header_109823as0920fs8_

#include <vector>
#include <ctime>
#include <cstdio>
#include <string>
#include <sstream>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace n47 {

namespace pt = boost::posix_time;


#define PV_YEN_PAIR 0.001
#define PV_DOLLAR_PAIR 0.00001

struct tick;

typedef std::vector<tick*> tick_data;

typedef std::vector<tick*>::iterator tick_data_iterator;

struct tick {

    tick()
    : td(pt::millisec(0)), ask(0.0), bid(0.0), askv(0.0), bidv(0.0)
    {}

    tick(pt::ptime epoch_, pt::time_duration ms, float a, float b, float av, float bv)
    : epoch(epoch_), td(ms), ask(a), bid(b), askv(av), bidv(bv)
    {}

    tick(const tick& rhs) {
        epoch = rhs.epoch;
        td = rhs.td;
        ask =  rhs.ask;
        bid = rhs.bid;
        askv = rhs.askv;
        bidv = rhs.bidv;
    }

    std::string str() const {
        std::stringstream strm;
        strm << epoch << ", " << td.total_milliseconds()
             << ask << ", " << askv << ", "
             << bid << ", " << bidv;
        return strm.str();
    }

    pt::ptime epoch;
    pt::time_duration td;
    float ask, bid;
    float askv, bidv;
};


struct BigEndian {};
struct LittleEndian {};

template <typename T, class endian>
struct bytesTo {
    T operator()(const unsigned char *buffer);
};

template <typename T>
struct bytesTo<T, BigEndian>{
    T operator()(const unsigned char *buffer) {
        T value;
        size_t index;
        for (index = sizeof(T); index > 0; index--) {
            ((unsigned char*) &value)[ sizeof(T) - index ]  =  *(buffer + index - 1);
        }
        return value;
    }
};

template <typename U>
struct bytesTo<U, LittleEndian>{
    U operator()(const unsigned char *buffer) {
        U value;
        size_t index;
        for (index = 0; index < sizeof(U); index++) {
            ((unsigned char*) &value)[ index ] = *(buffer + index);
        }
        return value;
    }
};


tick* tickFromBuffer(
        unsigned char *buffer, pt::ptime epoch, float digits, size_t offset=0);


tick_data* read_bin(
        unsigned char *buffer, size_t buffer_size, pt::ptime epoch, float point_value);


tick_data* read_bi5(
        unsigned char *lzma_buffer, size_t lzma_buffer_size, pt::ptime epoch,
        float point_value, size_t &bytes_read);


tick_data* read(
        const char *filename, pt::ptime epoch, float point_value, size_t &bytes_read);

} // namespace n47

#endif
