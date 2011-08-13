#ifndef RECEIVEREGISTERS_HH
#define RECEIVEREGISTERS_HH

#include "NowindTypes.hh"
#include <vector>

namespace nwhost {

class NowindHostSupport;

class ReceiveRegisters
{
public:
	ReceiveRegisters();
	virtual ~ReceiveRegisters();
	void initialize(NowindHostSupport* aSupport);

    void send();
    void ack(byte tail);
    bool isDone() const;
    
    void clear();
    void setA(byte data);
    void setF(byte data);
    void setBC(word data);
    void setDE(word data);
    void setHL(word data);
        
private:
    void sendData();
	word processedData;
	word transferSize;
	byte header;
	int errors;

    NowindHostSupport* nwhSupport;
    std::vector<byte> buffer;       // work buffer for current tranfer
	bool done;
};

} // namespace nwhost

#endif // RECEIVEREGISTERS_HH
