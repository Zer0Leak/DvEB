
#include <climits>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <cmath>
#include <iomanip>
#include <fstream>
#include <ostream>

#include <boost/log/trivial.hpp>

#include "bigdata/statistc.hh"
#include "bigdata/properties.hh"
#include "bigdata/util.hh"

namespace bigdata {
namespace statistic {

static __thread Sample * temp_sample_ = NULL;

double mean(int64_t sum, int64_t count) {
    if (count == 0) return 0.0;
    return static_cast<double>(sum) / static_cast<double>(count);
}

// s2 = AVG(x^2) - AVG(x)^2
// s2 = ( SUM (x^2) - SUM(x)^2 / n ) / (n - 1)

double variance(int64_t sum, int64_t count, int64_t sum2) {  // TODO: use Sample Variance here ?
    if (count <= 0) return 0;
    double a = static_cast<double>(sum2);
    double b = static_cast<double>(sum * sum) / count;
    return (a - b) / static_cast<double>(count);
}

double variance_sample(int64_t sum, int64_t count, int64_t sum2) {  // TODO: use Sample Variance here ?
    if (count <= 1) return 0;
    double a = static_cast<double>(sum2);
    double b = static_cast<double>(sum * sum) / count;
    return (a - b) / static_cast<double>(count-1);
}

double stdev(int64_t sum, int64_t count, int64_t sum2) {
    auto var = std::abs(variance(sum, count, sum2));
    return sqrt(var);
}

double stdev_sample(int64_t sum, int64_t count, int64_t sum2) {
    auto var = std::abs(variance_sample(sum, count, sum2));
    return sqrt(var);
}

double error(double stdev, int64_t count) {
    if (count == 0) return 0;
    return stdev / sqrt(count);
}

std::shared_ptr<Sample> include_thread_sample() {
    if (temp_sample_ != NULL) {
        return get_thread_sample();
    }
    temp_sample_ = new Sample();
    return std::shared_ptr<Sample>(temp_sample_, [](Sample*)
    {
        delete temp_sample_;
        temp_sample_ = NULL;
    });
}

std::shared_ptr<Sample> get_thread_sample() {
    if (temp_sample_ != NULL) {
        return std::shared_ptr<Sample>(temp_sample_, [](Sample*){});
    }
    return nullptr;
}

void thread_sample_set_level(bitscnt_t universe) {
    auto sample = get_thread_sample();
    if (sample != NULL) {
        sample->set_level(universe);
    }
}

void thread_sample_update(const Sample & sample) {
    auto s = get_thread_sample();
    if (s != NULL) {
        s->update(sample);
    }
}


std::unordered_map<pthread_t, Statistic> Statistic::registry_;
std::mutex Statistic::registry_mutex_;
std::unordered_map<TreeRpc::Method,
         std::map<int32_t, // level
            std::map<int32_t,  // iterations
             std::vector<Summary> > >
                , TreeRpc::MethodHash > Statistic::summaries_;

Sample::Sample() {
    zero();
}

void Sample::zero() {
    duration = 0;
    rmsg_all = 0;
    rmsg_timeout = 0;
    deeper_level = UINT32_MAX;
}

size_t Sample::write(FILE *stream) const {
    size_t nbytes = 0;
    size_t ret = 0;

    ret = fwrite(&rmsg_all, sizeof(rmsg_all), 1, stream);
    if (ret != 1) {
        throw std::runtime_error("Can't write rmsg_count");
    }
    nbytes += sizeof(rmsg_all);

    ret = fwrite(&rmsg_timeout, sizeof(rmsg_timeout), 1, stream);
    if (ret != 1) {
        throw std::runtime_error("Can't write rmsg_timeout_count");
    }
    nbytes += sizeof(rmsg_timeout);

    ret = fwrite(&deeper_level, sizeof(deeper_level), 1, stream);
    if (ret != 1) {
        throw std::runtime_error("Can't write deeper_level");
    }
    nbytes += sizeof(deeper_level);

    return nbytes;
}

size_t Sample::read(FILE *stream) {
    size_t nbytes = 0;
    size_t ret = 0;

    ret = fread(&rmsg_all, sizeof(rmsg_all), 1, stream);
    if (ret != 1) {
        throw std::runtime_error("Can't read rmsg_count");
    }
    nbytes += sizeof(rmsg_all);

    ret = fread(&rmsg_timeout, sizeof(rmsg_timeout), 1, stream);
    if (ret != 1) {
        throw std::runtime_error("Can't read rmsg_timeout_count");
    }
    nbytes += sizeof(rmsg_timeout);

    ret = fread(&deeper_level, sizeof(deeper_level), 1, stream);
    if (ret != 1) {
        throw std::runtime_error("Can't read deeper_level");
    }
    nbytes += sizeof(deeper_level);

    return nbytes;
}

SummaryGroupData::SummaryGroupData() {
    reset();
}

void SummaryGroupData::reset() {
    sum = 0;
    sum2 = 0;

    min = 0;
    max = 0;

    count = 0;
}

std::tuple<double, double, double> SummaryGroupData::calc() const {
    auto mean = statistic::mean(sum, count);
    auto stdev = statistic::stdev(sum, count, sum2);
    auto cv = mean != 0.0 ? stdev*100.0/mean : 0.0;
    return std::make_tuple(mean, stdev, cv);
}

void SummaryGroupData::update(int64_t val) {
    sum += val;
    sum2 += val * val;
    if (val < min) min = val;
    if (val > max) max = val;
    count++;
}

Summary::Summary() {}


Statistic::~Statistic() {
}

bool Statistic::included(pthread_t id) {
    std::lock_guard<std::mutex> lk(registry_mutex_);

    return registry_.find(id) != registry_.end();
}

const Statistic & Statistic::include(pthread_t id) {
    std::lock_guard<std::mutex> lk(registry_mutex_);

    return registry_[id];
}

void Statistic::remove(pthread_t id) {
    std::lock_guard<std::mutex> lk(registry_mutex_);

    registry_.erase(id);
}

Statistic & Statistic::get(pthread_t id) {
    std::lock_guard<std::mutex> lk(registry_mutex_);

    auto it_tid_st = registry_.find(id);

    if (it_tid_st != registry_.end()) {
        return it_tid_st->second;
    }

    throw std::runtime_error("Statistic " + std::to_string(id) + " not included");
}

void Statistic::zero(pthread_t id) {
    std::lock_guard<std::mutex> lk(registry_mutex_);

    auto it_tid_st = registry_.find(id);

    if (it_tid_st != registry_.end()) {
        it_tid_st->second.zero_intl();
    }
}

void Statistic::add(const Sample & sample, TreeRpc::Method method, pthread_t id) {
    std::lock_guard<std::mutex> lk(registry_mutex_);

    auto it_tid_st = registry_.find(id);

    if (it_tid_st != registry_.end()) {
        BOOST_LOG_TRIVIAL(trace) << "Adding to statistics " << id;
        it_tid_st->second.add_intl(sample, method);
    }
}

void Statistic::zero_intl() {
    std::lock_guard<std::mutex> lk(mutex_);

    statistic_.clear();
}

void Statistic::add_intl(const Sample & sample, TreeRpc::Method method) {
    std::lock_guard<std::mutex> lk(mutex_);

    auto & meth_val = statistic_[method];

    auto & sum = meth_val[sample.deeper_level];

    BOOST_LOG_TRIVIAL(trace) << "Method " << method << " adding sample " << sample;

    sum.first.all.update(sample.duration);
    sum.first.rmsg_all.update(sample.rmsg_all);
    if (sample.rmsg_timeout == 0) {
        sum.first.nt.update(sample.duration);
        sum.first.rmsg_nt.update(sample.rmsg_all);
    }

    if (Properties::get_instance()->statistics_summary_only() == false) {
        sum.second.push_back(sample);
    }
}


void ouput(std::ostream& stream, const SummaryGroupData & s, std::string indent="         ") {
    auto msv = s.calc();
    auto mean = std::get<0>(msv);
    auto stddev = std::get<1>(msv);
    auto cv = std::get<2>(msv);

    stream << indent << "sum2: " << format_with_commas(s.sum2) << " us" << std::endl;
    stream << indent << "sum: " << format_with_commas(s.sum) << " us" << std::endl;
    stream << indent << "max: " << format_with_commas(s.max) << " us" << std::endl;
    stream << indent << "min: " << format_with_commas(s.min) << " us" << std::endl;
    stream << indent << "min: " << format_with_commas(s.count) << " us" << std::endl;
    stream << indent << "mean: " << format_with_commas(mean,2) << " us" << std::endl;
    stream << indent << "stddev: " << format_with_commas(stddev,2) << " us" << std::endl;
    stream << indent << "CV        : " << format_with_commas(cv,2) << " %" << std::endl;
}

std::ostream& operator <<(std::ostream& stream, const Summary & s) {

    stream << "       all:";
    ouput(stream, s.all, "         ");
    stream << "       nt:";
    ouput(stream, s.nt, "         ");
    stream << "       rmsg_all:";
    ouput(stream, s.rmsg_all, "         ");
    stream << "       rmsg_nt:";
    ouput(stream, s.rmsg_nt, "         ");

    return stream;
}

std::ostream& operator <<(std::ostream& stream, const Sample & sample) {
    //    stream << '(' << sample.duration << ',' << sample.deeper_level << ',' << sample.rmsg_all << ',' << sample.rmsg_timeout << "); ";

    return stream;
}

struct SummaryData {
    int64_t sum2;
    int64_t sum;
    int64_t count;
    double mean;
    double stdev;
    double cv;
};


std::ostream& operator <<(std::ostream& stream, const Statistic & statistic) {
    std::lock_guard<std::mutex> lk(statistic.mutex_);

    bool first = true;
    for (const auto & meth_map_lvl_summ_slps : statistic.statistic_) {
        if (first) {
            stream << std::endl << "method: " << meth_map_lvl_summ_slps.first << std::endl;
            first = false;
        } else {
            stream << "method: " << meth_map_lvl_summ_slps.first << std::endl;
        }

        for (const auto & lvl_summ_slps : meth_map_lvl_summ_slps.second) {
            stream << "    level summary: " << lvl_summ_slps.first << std::endl;
            stream << lvl_summ_slps.second.first;

            for (const auto & sample : lvl_summ_slps.second.second) {
                stream << sample;
            }
            stream << std::endl;
        }
    }

    return stream;
}

static void calc_nol(std::pair<Summary, std::vector<Sample>> & summary_samples) {
    const auto & samples = summary_samples.second;
    auto & summary = summary_samples.first;

    auto calc_all = summary.all.calc();
    auto mean_all = std::get<0>(calc_all);
    auto stdev_all = std::get<1>(calc_all);

    auto calc_nt = summary.nt.calc();
    auto mean_nt = std::get<0>(calc_nt);
    auto stdev_nt = std::get<1>(calc_nt);

    summary.all_nol.reset();
    summary.nt_nol.reset();

    for(const auto & sample : samples) {

        if ((sample.duration >= (mean_all-(stdev_all*3))) && (sample.duration <= (mean_all+(stdev_all*3)))) {
            summary.all_nol.update(sample.duration);
        }

        if (sample.rmsg_timeout == 0) {
            if ((sample.duration >= (mean_nt-(stdev_nt*3))) && (sample.duration <= (mean_nt+(stdev_nt*3)))) {
                summary.nt_nol.update(sample.duration);
            }
        }
    }
}

void Statistic::save(int32_t iter) {
    std::lock_guard<std::mutex> lk(registry_mutex_);

    for (auto & meth_rest : statistic_) {
        auto & method = meth_rest.first;
        auto & summ_meth_val = summaries_[method];
        for (auto & lvl_rest : meth_rest.second) {

            auto & summary = lvl_rest.second.first;
            auto & level = lvl_rest.first;

            auto & summ_lvl_val = summ_meth_val[level];
            auto & summ_iter_val = summ_lvl_val[iter];

            calc_nol(lvl_rest.second);

            summ_iter_val.push_back(summary);
        }
    }
}

static void save_line(const std::string & what, const std::string & mode, TreeRpc::Method method, std::fstream & fs, int32_t level, int32_t control_lvl, const std::map<int32_t,std::vector<Summary> > & p_lvl_val) {
    const int32_t width = 10;
    const int32_t precision = 0;

    while (level > ++control_lvl) {
        fs << std::setw(2) << std::right << control_lvl << std::setw(7) << std::right << what << std::endl;
    }

    fs << std::setw(2) << std::right << level << std::setw(7) << std::right << what;

    int32_t control_iter = 0;
    for(const auto & p_iter : p_lvl_val) {
        const auto & iter = p_iter.first;
        const auto & summ_vec = p_iter.second;

        if (summ_vec.size() > 1) {
            BOOST_LOG_TRIVIAL(error) << "More tha 1 summary for method " << method << " level " << level << " iter " << iter;
        }

        while(iter > ++control_iter) {
            fs << std::setw(width+1) << std::left << ";";
        }

        if (summ_vec.size() == 0) {
            fs << std::setw(width+1) << std::left << ";";
        } else {
            const SummaryGroupData * sgd = NULL;
            if (mode == "all") {
                sgd = &summ_vec[0].all;
            } else if (mode == "nt") {
                sgd = &summ_vec[0].nt;
            } else if (mode == "all_nol") {
                sgd = &summ_vec[0].all_nol;
            } else if (mode == "nt_nol") {
                sgd = &summ_vec[0].nt_nol;
            } else {
                BOOST_LOG_TRIVIAL(error) << "Unknown mode " << mode;
                return;
            }

            if (sgd->count > 0) {
                double mean;
                double stdev;
                auto count = sgd->count;

                std::tie(mean, stdev, std::ignore) = sgd->calc();

                auto err = error(stdev, count);

                if (what == "mean") {
                    fs << ";" << std::setw(width) << std::right << std::fixed << std::setprecision(precision) << mean;
                } else if (what == "count") {
                    fs << ";" << std::setw(width) << std::right << std::fixed << std::setprecision(precision) << count;
                } else if (what == "stdev") {
                    fs << ";" << std::setw(width) << std::right << std::fixed << std::setprecision(precision) << stdev;
                } else {
                    fs << ";" << std::setw(width) << std::right << std::fixed << std::setprecision(precision) << err;
                }
            } else {
                fs << std::setw(width+1) << std::left << ";";
            }


        }
    }

    fs << std::endl;
}

void Statistic::save(const std::string & folder) {
    std::lock_guard<std::mutex> lk(registry_mutex_);

    for (const std::string & mode : {"all", "nt", "all_nol", "nt_nol"}) {

        for(const auto & p_meth : summaries_) {
            const auto & method = p_meth.first;
            const auto & p_meth_val = p_meth.second;

            std::ostringstream filename;
            filename << method;

            std::string folder_del;
            if (folder.size() > 0 && folder.back() != '/') {
                folder_del = "/";
            }

            std::fstream fs;
            fs.open (folder + folder_del + mode + "_" + filename.str(), std::fstream::out | std::fstream::trunc);
            if (fs.is_open() == false) {
                throw std::runtime_error("Can't open file " + filename.str());
            }

            int32_t control_lvl = 0;
            for(const auto & p_lvl : p_meth_val) {
                const auto & level = p_lvl.first;
                const auto & p_lvl_val = p_lvl.second;

                save_line("mean", mode, method, fs, level, control_lvl, p_lvl_val);
                save_line("count", mode, method, fs, level, control_lvl, p_lvl_val);
                save_line("stdev", mode, method, fs, level, control_lvl, p_lvl_val);
                save_line("error", mode, method, fs, level, control_lvl, p_lvl_val);
                fs << std::endl;

                control_lvl = level;
            }

            fs.close();
        }
    }
}


}
}
