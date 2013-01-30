#include "PyNwhostService.h"
#include <boost/bind.hpp>

void PyNwhostService::start()
{
	nowind::initialize();
	// start new thread for NwhostService::hostImage
	m_thread = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&PyNwhostService::hostImage, this)));
}

void PyNwhostService::stop()
{
	setRunningFalse();
	m_thread->join();
}
