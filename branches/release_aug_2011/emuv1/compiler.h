// add this to pass the Ultimate++ option 

#ifndef WIN32
#ifdef flagWIN32
 #define WIN32
#endif
#endif

#ifdef _MSC_VER
	// ??
	#pragma warning (disable: 4309)
	// no 
	#pragma warning (disable: 4291)
	#define NW_OPENMODE std::ios_base::openmode
#else
	#define NW_OPENMODE std::_Ios_Openmode
#endif



