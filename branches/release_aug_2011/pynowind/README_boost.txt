 
 ---=== Installation instructions =----
 
# WINDOWS 

- install Python 2.4 or newer (2.5 or 2.6 have been reported to work)
	current download link: http://www.python.org/ftp/python/2.6.2/python-2.6.2.msi
	see http://www.python.org/download/ if outdated.
	
- install boost
	download boost_1_38_setup.exe or newer from http://www.boostpro.com/download
	
install at least:

x Boost DateTime
x Boost Thread
x Boost Python

Typical include directories on windows:
	$(ProjectDir)..\general;$(ProjectDir)..\ftdx;$(ProjectDir)..\nowind;$(ProjectDir)..\nwhost;.;C:\Program Files\boost\boost_1_38;C:\Python26\include
Typical addition linker directories on windows:
	C:\Program Files\boost\boost_1_38\lib;C:\Python26\libs

### OTHER POSIX PLATFORMS ###

(tested with Ubuntu 8.04 LTS)

- todo: test on 'Mac Os Leopard'

Either find corresponding boost packages too install
or compile from sourcecode (can be downloaded from http://www.boost.org/) 

To test your Boost::Python installation, try to build: nowindlibraries/setup/boost_python/example.cpp using build.sh
if this boost 'hello world' fails, something is wrong with the python or boost packages.

on Ubuntu 8.04 LTS I used:

- libboost-date-time1.34.1
- libboost-python1.34.1
- libboost-python-dev
- libboost-thread1.34.1
- libboost-thread-dev

- 

