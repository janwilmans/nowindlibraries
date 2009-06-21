#include "libnowind.h"
#include "PyNwhostService.h"

using namespace nowind;

#include <boost/python.hpp>
using namespace boost::python;

BOOST_PYTHON_MODULE(pynowind)
{
	/*
    class_<NwhostService>("NwhostService")
        .def("setImage", &NwhostService::setImage)
    ;
    

	class_<PyNwhostService, bases<NwhostService> >("PyNwhostService")
        .def("start", &PyNwhostService::start)
        .def("stop", &PyNwhostService::stop)
    ;
    */

    class_<PyNwhostService>("PyNwhostService")
        .def("setImage", &PyNwhostService::setImage)
        .def("start", &PyNwhostService::start)
        .def("stop", &PyNwhostService::stop)
	;
}
