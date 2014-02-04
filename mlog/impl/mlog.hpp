/*
 * mlog.cpp
 *
 *  Created on: Aug 12, 2012
 *  Author: philipp
 */

#ifndef __MLOG_IPP__
#define __MLOG_IPP__

#include "../mlog.hpp"

namespace mlog {

//std::unique_ptr<logger> mlogger(new standard_logger());

// leaks to enable logging at shutdown, not deleted unless reset by user.
logger *mlogger = new standard_logger();

void reset(logger *log) {
	if( mlogger )
		delete mlogger;

	mlogger = log;
}

}

#endif
