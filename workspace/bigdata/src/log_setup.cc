

#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/severity_logger.hpp>

#include <boost/core/null_deleter.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/make_shared.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/sources/record_ostream.hpp>

#include <boost/thread/thread.hpp>
#include <boost/log/attributes/scoped_attribute.hpp>
#include <boost/log/attributes/current_thread_id.hpp>

#include <boost/log/attributes/clock.hpp>

#include <boost/log/support/date_time.hpp>

#include "bigdata/log_setup.hh"

namespace bigdata {

namespace log {

static void coloring_formatter(boost::log::record_view const& rec, boost::log::formatting_ostream& strm)
{
    auto severity = rec[boost::log::trivial::severity];
    if (severity)
    {
        // Set the color
        switch (severity.get())
        {
        case boost::log::trivial::info:
            strm << "\033[32m";
            break;
        case boost::log::trivial::warning:
            strm << "\033[33m";
            break;
        case boost::log::trivial::error:
        case boost::log::trivial::fatal:
            strm << "\033[31m";
            break;
        default:
            break;
        }
    }

    // auto a = boost::log::extract< boost::log::attributes::basic_clock< boost::log::attributes::local_time_traits > >("TimeStamp", rec);
    const boost::posix_time::ptime &pt = *boost::log::extract< boost::posix_time::ptime >("TimeStamp", rec);
    strm << boost::posix_time::ptime(pt);
    strm << '[' << boost::log::extract< boost::thread::id >("ThreadID", rec) << ']';

    strm
         << " <" << rec[boost::log::trivial::severity]
         << "> " << rec[boost::log::expressions::smessage];

    if (severity)
    {
        // Restore the default color
        strm << "\033[0m";
    }

}


void setup(boost::log::trivial::severity_level level) {

    boost::shared_ptr< boost::log::core > core = boost::log::core::get();

    // Create a backend and attach a couple of streams to it
    boost::shared_ptr< boost::log::sinks::text_ostream_backend > backend =
            boost::make_shared< boost::log::sinks::text_ostream_backend >();
    backend->add_stream(
                boost::shared_ptr< std::ostream >(&std::clog, boost::null_deleter()));
    //backend->add_stream(
    //    boost::shared_ptr< std::ostream >(new std::ofstream("sample.log")));

    // Enable auto-flushing after each log record written
    backend->auto_flush(true);

    // Wrap it into the frontend and register in the core.
    // The backend requires synchronization in the frontend.
    typedef boost::log::sinks::synchronous_sink< boost::log::sinks::text_ostream_backend > sink_t;
    boost::shared_ptr< sink_t > sink(new sink_t(backend));

    // core->add_global_attribute("LineID", boost::log::attributes::counter< unsigned int >(1));
    core->add_global_attribute("TimeStamp", boost::log::attributes::local_clock());

    sink->set_formatter(&coloring_formatter);

    /*
    sink->set_formatter
            (
                boost::log::expressions::format("%1% <%2%> %3%")
                % boost::log::expressions::format_date_time< boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d %H:%M:%S.%f")
                % boost::log::trivial::severity
                % boost::log::expressions::smessage
                );
                */


    core->add_sink(sink);


    core->set_filter(
                boost::log::trivial::severity >= level
                );

    boost::log::add_common_attributes();
}

}
}
