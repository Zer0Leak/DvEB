
#ifndef BIGDATA_STATISTIC_HH_
#define BIGDATA_STATISTIC_HH_

#include <cstdint>

#include <memory>
#include <pthread.h>
#include <map>
#include <unordered_map>
#include <mutex>
#include <vector>
#include <ostream>

#include "bigdata/iserializable.hh"
#include "bigdata/tree_rpc.hh"
#include "bigdata/util.hh"

namespace bigdata {
namespace statistic {

extern double mean(int64_t sum, int64_t count);
extern double variance(int64_t sum, int64_t count, int64_t sum2);
extern double variance_sample(int64_t sum, int64_t count, int64_t sum2);
extern double stdev(int64_t sum, int64_t count, int64_t sum2);
extern double stdev_sample(int64_t sum, int64_t count, int64_t sum2);
extern double error(double stdev, int64_t count);

extern std::shared_ptr<Sample> include_thread_sample();
extern std::shared_ptr<Sample> get_thread_sample();
extern void thread_sample_set_level(bitscnt_t universe);
extern void thread_sample_update(const Sample & sample);

struct Sample : public ISerializable {
    Sample();
    ~Sample() = default;

    void zero();
    void update(const Sample & sample) {
        rmsg_all += sample.rmsg_all;
        rmsg_timeout += sample.rmsg_timeout;
        if (sample.deeper_level < deeper_level) {
            deeper_level = sample.deeper_level;
        }
    }

    void set_level(bitscnt_t universe) {
        if (universe < deeper_level) {
            deeper_level = universe;
        }
    }

    void normalize_level(bitscnt_t root_universe) {
        auto base_level = bigdata::log2(root_universe);
        deeper_level = bigdata::log2(deeper_level);
        if (base_level >= deeper_level) {
            deeper_level = base_level - deeper_level;
        }
    }

    virtual size_t write(FILE *stream) const;
    virtual size_t read(FILE *stream);

    int64_t duration;
    int64_t rmsg_all;
    int64_t rmsg_timeout;
    uint32_t deeper_level;
};

struct SummaryGroupData {
    SummaryGroupData();
    void reset();
    int64_t sum;
    int64_t sum2;
    int64_t min;
    int64_t max;
    int64_t count;

    // mean, stdev, cv
    std::tuple<double, double, double> calc() const;
    void update(int64_t val);
};

struct Summary {
    Summary();

    SummaryGroupData all;
    SummaryGroupData nt;

    SummaryGroupData all_nol;
    SummaryGroupData nt_nol;

    SummaryGroupData rmsg_all;
    SummaryGroupData rmsg_nt;
};

class Statistic {
public:
    Statistic() = default;
    ~Statistic();

    static Statistic & get(pthread_t id = pthread_self());

    static bool included(pthread_t id = pthread_self());
    static const Statistic & include(pthread_t id = pthread_self());
    static void remove(pthread_t id = pthread_self());
    static void zero(pthread_t id = pthread_self());
    static void add(const Sample & sample, TreeRpc::Method method, pthread_t id = pthread_self());

    void save(int32_t inter);

    static void save(const std::string & folder);

    const std::unordered_map<TreeRpc::Method,
            std::map<int32_t /* level */,
              std::pair<Summary, std::vector<Sample>>>, TreeRpc::MethodHash > & statistic(pthread_t id = pthread_self()) { return statistic_; }

private:
    static std::unordered_map<pthread_t, Statistic> registry_;
    static std::mutex registry_mutex_;

    std::unordered_map<TreeRpc::Method,
        std::map<int32_t, // level
            std::pair<Summary, std::vector<Sample>>>, TreeRpc::MethodHash > statistic_;

    mutable std::mutex mutex_;
    void add_intl(const Sample & sample, TreeRpc::Method method);
    void zero_intl();

    friend std::ostream& operator <<(std::ostream& stream, const Statistic& statistics);

    static std::unordered_map<TreeRpc::Method,
             std::map<int32_t, // level
                std::map<int32_t,  // iterations
                 std::vector<Summary> > >
                    , TreeRpc::MethodHash > summaries_;

};

extern std::ostream& operator <<(std::ostream& stream, const Summary & summary);

extern std::ostream& operator <<(std::ostream& stream, const Sample & sample);

extern std::ostream& operator <<(std::ostream& stream, const Statistic& statistics);

}
}

#endif  // BIGDATA_STATISTIC_HH_
