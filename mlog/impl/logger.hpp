/*
*
*	Author: Philipp Zschoche
*
*/

#ifndef __LOGGER_IPP__
#define __LOGGER_IPP__

#include "../logger.hpp"
#include <boost/random.hpp>
#include <cstdio>
#if _MSC_VER
#define snprintf _snprintf_s
#endif

#include <sstream>

namespace mlog {


template<typename T>
	std::string thread_id_to_string(T&& thread_id) {
		std::stringstream ss;
	ss << thread_id;
	return ss.str();
}


log_metadata::log_metadata(mlog_level &&lvl, short session_id, bool _use_time,
			   bool _use_thread_id)
    : use_time(std::move(_use_time)), use_thread_id(std::move(_use_thread_id)),
      use_position(false), level(std::move(lvl)),
      session_id(std::move(session_id)) {
	if (use_time)
		time = clocks::now();

	if (use_thread_id)
		thread_id = THREAD_GET_ID();
}

log_metadata::log_metadata(mlog_level &&lvl, short session_id, bool _use_time,
			   bool _use_thread_id, const log_position &_position,
			   bool _use_position)
    : use_time(std::move(_use_time)), use_thread_id(std::move(_use_thread_id)),
      use_position(_use_position), level(std::move(lvl)),
      session_id(std::move(session_id)), position(_position) {
	if (use_time)
		time = clocks::now();

	if (use_thread_id)
		thread_id = THREAD_GET_ID();
}

log_metadata::log_metadata(mlog_level &&lvl, short session_id, bool _use_time,
			   bool _use_thread_id, log_position &&_position,
			   bool _use_position)
    : use_time(std::move(_use_time)), use_thread_id(std::move(_use_thread_id)),
      use_position(_use_position), level(std::move(lvl)),
      session_id(std::move(session_id)), position(std::move(_position)) {
	if (use_time)
		time = clocks::now();

	if (use_thread_id)
		thread_id = THREAD_GET_ID();
}

std::string log_metadata::to_string(const std::string& end_string) const {
	char buffer[1024];
	if (use_time) {

		std::time_t timet = clocks::to_time_t(time);

		unsigned long long ms =
		    std::chrono::duration_cast<std::chrono::nanoseconds>(
			time.time_since_epoch()).count() -
		    timet * 1000000000;

#if _MSC_VER
		tm tmm;
		localtime_s(&tmm, &timet);
		tm *tm = &tmm;
#else
		std::tm *tm = std::localtime(&timet);
#endif

		if (use_thread_id) {
			if (use_position) // 2012-11-02 15:24:04.345
					  // [file:line_number
					  // 24-0x7fff72ca8180]{warning}:
				snprintf(
				    buffer, sizeof(buffer),
				    "%04i-%02i-%02i %02i:%02i:%02i.%llu "
				    "[%s:%li %02i-%s]{%s}: ",
				    1900 + tm->tm_year, tm->tm_mon, tm->tm_mday,
				    tm->tm_hour, tm->tm_min, tm->tm_sec, ms,
				    position.filename.c_str(),
				    position.line_number, session_id,
				    thread_id_to_string(thread_id)
					.c_str(),
				    level_to_string(level).c_str());
			else // 2012-11-02 15:24:04.345
			     // [24-0x7fff72ca8180]{warning}:
				snprintf(buffer, sizeof(buffer),
					 "%04i-%02i-%02i %02i:%02i:%02i.%llu "
					 "[%02i-%s]{%s}: ",
					 1900 + tm->tm_year, tm->tm_mon,
					 tm->tm_mday, tm->tm_hour, tm->tm_min,
					 tm->tm_sec, ms, session_id,
					 thread_id_to_string(
					     thread_id).c_str(),
					 level_to_string(level).c_str());
		} else // 2012-11-02 15:24:04.345 [24]{warning}:
		{
			if (use_position)
				snprintf(
				    buffer, sizeof(buffer),
				    "%04i-%02i-%02i %02i:%02i:%02i.%llu[%s:%li "
				    "%02i]{%s}: ",
				    1900 + tm->tm_year, tm->tm_mon, tm->tm_mday,
				    tm->tm_hour, tm->tm_min, tm->tm_sec, ms,
				    position.filename.c_str(),
				    position.line_number, session_id,
				    level_to_string(level).c_str());
			else
				snprintf(buffer, sizeof(buffer),
					 "%04i-%02i-%02i "
					 "%02i:%02i:%02i.%llu[%02i]{%s}: ",
					 1900 + tm->tm_year, tm->tm_mon,
					 tm->tm_mday, tm->tm_hour, tm->tm_min,
					 tm->tm_sec, ms, session_id,
					 level_to_string(level).c_str());
		}
	} else if (use_thread_id) //[24-0x7fff72ca8180]{warning}:
	{
		if (use_position)
			snprintf(
			    buffer, sizeof(buffer), "[%s:%li %02i-%s]{%s}: ",
			    position.filename.c_str(), position.line_number,
			    session_id,
			    thread_id_to_string(thread_id).c_str(),
			    level_to_string(level).c_str());
		else
			snprintf(buffer, sizeof(buffer), "[%02i-%s]{%s}: ",
				 session_id, thread_id_to_string(
						 thread_id).c_str(),
				 level_to_string(level).c_str());
	} else if (use_position) {
		snprintf(buffer, sizeof(buffer), "[%s:%li %02i]{%s}: ",
			 position.filename.c_str(), position.line_number,
			 session_id, level_to_string(level).c_str());
	} else //[24]{warning}:
	{
		snprintf(buffer, sizeof(buffer), "[%02i]{%s}: ", session_id,
			 level_to_string(level).c_str());
	}
	
	if(end_string.empty()) {
		return std::string(buffer);
	} else {
		std::size_t len = strlen(buffer);
		std::string result;
		result.resize(len + end_string.size());
		char* rbuf = const_cast<char*>(result.c_str());
		memcpy(rbuf, buffer, len);
		memcpy(&rbuf[len], end_string.c_str(), end_string.size());
		return result;
	}
}

std::ostream &log_metadata::output(std::ostream &stream) const {

	stream << to_string();
	return stream;
}

logger::logger()
    : m_use_time(false), m_use_thread_id(false), m_use_position(false) {
	using namespace boost;
	using namespace boost::random;
	mt19937 rng(std::time(0));

#ifdef BOOST_HAS_TR1_RANDOM
	uniform_int_distribution<> six(0, 100);
	m_session = six(rng);
#elseif
	boost::uniform_int<> six(0, 100);
	boost::variate_generator<boost::mt19937 &, boost::uniform_int<> > die(
	    rng, six);
	m_session = die();
#endif
}

logger::~logger() {}

} /* mlog */

#endif
