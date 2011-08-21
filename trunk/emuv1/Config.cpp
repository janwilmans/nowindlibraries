// Config.cc

#include "stdio.h"
#include <string>
#include "Config.h"
#include "Debug.h"

using namespace std;

Config::Config() {
        
        DBERR("Config constructor...\n");
		string filename("nowind.conf");

		conffile = new ifstream(filename.c_str());
		if (conffile->fail()) {
				DBERR("Error opening file %s, using internal defaults!\n", filename.c_str());
		}
 
		/* configuration defaults */
		msxVersion = getIntValueOf("msxversion", 1);

}

int Config::getIntValueOf(string , int defaultValue) {

	return defaultValue;
}

Config * Config::Instance() {

		/* implies singleton class */

		static Config deInstantie;
		return &deInstantie;
}

Config::~Config() {

		// destructor
		delete(conffile); // waarom wordt deze niet vrijgemaakt JAN?
		DBERR("Config destroyed.\n");
}

