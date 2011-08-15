#ifndef _pynowind_PyNwhostService_h_
#define _pynowind_PyNwhostService_h_

#include "libnowind.h"
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>

using namespace boost;

class PyNwhostService: public nowind::NwhostService {
	
	public:
		void start();
    	void stop();
    private:
	    boost::shared_ptr<boost::thread> m_thread;

    
};

#endif
