//
// Created by yang on 17-5-17.
//

#ifndef HTTPSERVER_LOGGER_H
#define HTTPSERVER_LOGGER_H

#include <map>
#include <iostream>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/common.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/attributes/timer.hpp>
#include <boost/log/attributes/named_scope.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/sources/severity_feature.hpp>
#include <boost/log/sources/severity_logger.hpp>

#include <vector>
#include <atomic>
#include <thread>
#include <memory>
#include <fstream>
#include <boost/shared_ptr.hpp>

namespace logging = boost::log;
namespace sinks = boost::log::sinks;
namespace attrs = boost::log::attributes;
namespace src = boost::log::sources;
namespace expr = boost::log::expressions;
namespace keywords = boost::log::keywords;
namespace sinks = boost::log::sinks;

//设置日志等级
enum severity_level
{
	normal,
	notification,
	warning,
	error,
	critical,
	debug
};

//日志等级的输出
template< typename CharT, typename TraitsT >
inline std::basic_ostream< CharT, TraitsT >& operator<< (
	std::basic_ostream< CharT, TraitsT >& strm, severity_level lvl)
{
	static const char* const str[] =
		{
			"normal",
			"notification",
			"warning",
			"error",
			"critical",
		    "debug"
		};
	if (static_cast< std::size_t >(lvl) < (sizeof(str) / sizeof(*str)))
		strm << str[lvl];
	else
		strm << static_cast< int >(lvl);
	return strm;
}

//using text_sink = sinks::synchronous_sink< sinks::basic_text_ostream_backend >;

class Logger {
public:

	//typedef sinks::synchronous_sink< sinks::text_ostream_backend > text_sink;

	static void init(std::string logPath = "../");
	static void LogNormal(std::string msg) __wur __attribute_deprecated__;
	static void LogNotification(std::string msg);
	static void LogWarning(std::string msg);
	static void LogError(std::string msg);
	static void LogCritical(std::string msg);
	static void LogDebug(std::string msg);


private:
	static void setLogPath(std::string logPath);

	static void setFilter();

	static src::logger lg;
	static src::severity_logger< severity_level > slg;
	static std::atomic_flag atomic_bool;

};


#endif //HTTPSERVER_LOGGER_H



