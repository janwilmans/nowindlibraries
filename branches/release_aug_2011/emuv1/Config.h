//! Config.h
#ifndef CONFIG_H
#define CONFIG_H

#include <iostream>
#include <fstream>
#include <string>

class Config {

private:

						Config();
		int             getIntValueOf(std::string name, int defaultValue);
public:

		std::ifstream   *conffile;
		static Config   *Instance();
						~Config();

		/* configuration options */
		int             msxVersion;

};
#endif

