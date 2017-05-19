//
// Created by yang on 17-5-17.
//

#include "Logger.h"


src::logger Logger::lg;
src::severity_logger< severity_level> Logger::slg;	//新建logger实例 使用自定义的 severity
std::atomic_flag Logger::atomic_bool = ATOMIC_FLAG_INIT;    //对日志文件的原子操作



void Logger::init(std::string logPath ) {

	setLogPath(logPath);

	//设置常用属性
	logging::add_common_attributes();
	logging::core::get()->add_thread_attribute("Scope", attrs::named_scope());
	slg.add_attribute("Uptime", attrs::timer());
	BOOST_LOG_FUNCTION();
}


void Logger::setLogPath(std::string logPath = "../") {
	//if(logPath.back() != '/') logPath += '/';

	//新建一个输出到 console 的sink
	auto conSink = logging::add_console_log(std::clog, keywords::format = "%TimeStamp%: %Message%");

	//新建一个输出到文件的 sink
	auto fileSink = logging::add_file_log(
		keywords::open_mode = std::ios::app, /*追加写入*/
		keywords::file_name = logPath + "server_log_%N.log",
		keywords::rotation_size = 10 * 1024 * 1024,
		keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0),
		keywords::format =
			expr::format("%1% [%2%] [%3%] <%4%> (%5%) %6%")
			% expr::format_date_time< boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d, %H:%M:%S.%f")
			% expr::format_date_time< attrs::timer::value_type >("Uptime", "%O:%M:%S")
			% expr::format_named_scope("Scope", keywords::format = "%n (%f : %l)") /*这一项没有输出 待解决*/
			% expr::attr< severity_level >("Severity") 
			% expr::attr<boost::log::attributes::current_thread_id::value_type >("ThreadID")
			% expr::message

//	keywords::format = expr::stream
//			<< expr::attr<unsigned int>("LineID")
//			<< expr::format_date_time< boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d, %H:%M:%S.%f")
//			<< " [" << expr::format_date_time< attrs::timer::value_type >("Uptime", "%O:%M:%S")
//			<< "] [" << expr::format_named_scope("Scope", keywords::format = "%n (%f:%l)")
//			<< "] <" << expr::attr< severity_level >("Severity")
//			<< "> " << expr::message


	);

	//fileSink->locked_backend()->scan_for_files();
	fileSink->locked_backend()->auto_flush(true);   //立刻写入文件

	logging::core::get()->add_sink(conSink);  //注册sink
	logging::core::get()->add_sink(fileSink);

}




//设置日志过滤
void Logger::setFilter() {
	logging::core::get()->set_filter(
		logging::trivial::severity >= notification
	);
}

void Logger::LogNormal(std::string msg) {
	while(atomic_bool.test_and_set()) {     //检查atomic_flag并设置为 true  使用atomic_flag起到和加锁同样的效果（貌似性能更高）
		std::this_thread::yield();  //放弃当前CPU时间 然后等待调度重新竞争CPU 适合反复检查条件时使用
	}
	BOOST_LOG_SEV(slg, normal) << msg;
	atomic_bool.clear();    //设置atomic_flag
}

void Logger::LogNotification(std::string msg) {
	while(atomic_bool.test_and_set()) {
		std::this_thread::yield();
	}
	BOOST_LOG_SEV(slg, notification) << msg;
	atomic_bool.clear();
}

void Logger::LogWarning(std::string msg) {
	while(atomic_bool.test_and_set()) {
		std::this_thread::yield();
	}
	BOOST_LOG_SEV(slg, warning) << msg;
	atomic_bool.clear();
}

void Logger::LogError(std::string msg) {
	while(atomic_bool.test_and_set()) {
		std::this_thread::yield();
	}
	BOOST_LOG_SEV(slg, error) << msg;
	atomic_bool.clear();
}

void Logger::LogCritical(std::string msg) {
	while(atomic_bool.test_and_set()) {
		std::this_thread::yield();
	}
	BOOST_LOG_SEV(slg, critical) << msg;
	atomic_bool.clear();
}

void Logger::LogDebug(std::string msg) {
	while(atomic_bool.test_and_set()) {
		std::this_thread::yield();
	}
	BOOST_LOG_SEV(slg, debug) << msg;
	atomic_bool.clear();
}






