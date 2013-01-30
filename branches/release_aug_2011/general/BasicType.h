/**
 * @file BasicType.h
 *
 * @brief generic basic type class
 * @author Jan Wilmans <jw@dds.nl>  Copyright (C) 2009 Nowind
 *
 */

#ifndef BASICTYPE_H
#define BASICTYPE_H

#include "GeneralExports.h"

#include <string>

namespace general {

class BasicType;

class GEN_API BasicType
{
    enum basicType { eCommand, eAttribute };
    public:
    	static BasicType* newCommand() { return new BasicType(eCommand); }
    	static BasicType* newAttribute() { return new BasicType(eAttribute); }
    	
    	void set(std::string aName, int aValue);
    	
    private:
    	BasicType(basicType aType);
    	
    	basicType mType;
    	std::string mName;
    	int mNativeValue;
    protected:
    	~BasicType();
    	
};

} // namespace general 

#endif // BASICTYPE_H
