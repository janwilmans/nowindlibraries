
#include "libgeneral.h"
#include "BlockRead.hh"
#include "DataBlock.hh"
#include "NowindHostSupport.hh"

#include <cassert>

#define DBERR nwhSupport->debugMessage

namespace nwhost {

BlockRead::BlockRead()
{
}

void BlockRead::initialize(NowindHostSupport* aSupport)
{
    nwhSupport = aSupport;
}

BlockRead::~BlockRead()
{
}

bool BlockRead::isDone() const
{
    return done;
}

void BlockRead::init(word aStartAddress, word aSize, const std::vector <byte >& data, byte aReturnCode)
{
    //DBERR("BlockRead::init(startAddress: 0x%04x, size: 0x%04x\n", aStartAddress, aSize);
    startAddress = aStartAddress;
    transferSize = aSize;
    returnCode = aReturnCode;

    transferingToPage01 = (startAddress <= TWOBANKLIMIT);
    processedData = 0;
    done = false;
    
    // copy data to transfer
    buffer.clear();
    buffer = data;
    blockRead(startAddress, transferSize);
}

void BlockRead::blockRead(word startAddress, word size)
{
    //DBERR("BlockRead::blockRead, startAddress: 0x%04X, size: 0x%04X\n", startAddress, size);
    
    if (startAddress < TWOBANKLIMIT)
    {
        // notice the () around std::min, this is to prevent conflicts with min/max defines (if any, like in windows.h)
        word endAddress = (std::min)(TWOBANKLIMIT, startAddress + size);
        word transferSize = endAddress - startAddress;
        blockReadHelper(startAddress, transferSize);
    }
    else
    {
        blockReadHelper(startAddress, size);
    }  
    //DBERR("blockRead, processedData: 0x%04X\n", processedData);
}

void BlockRead::blockReadHelper(word startAddress, word size)
{
    //DBERR("BlockRead::blockReadHelper, size: 0x%02x, processedData: 0x%02x\n", size, processedData);
    
    // delete any blocks still in the dataBlockQueue (unacknowleged by msx, could be caused by timeout or disconnect)
    for(unsigned int i=0; i< dataBlockQueue.size(); i++)
    {        
        delete dataBlockQueue[i];
    }
    dataBlockQueue.clear();    
    
    word address = startAddress;
    unsigned int offset = processedData;      // an offset within 'const vector <byte >& data'

    if (size < HARDCODED_READ_DATABLOCK_SIZE)
    {   
        // just 1 slow block
        //DBERR("create slow block starting at: 0x%04X, size: 0x%02x\n", address, size);
        dataBlockQueue.push_front(new DataBlock(0, buffer, offset, address, size));
        nwhSupport->sendHeader();
        nwhSupport->send(BLOCKREAD_SLOWTRANSFER);
        nwhSupport->send16(startAddress);
        nwhSupport->send16(size);   // amount of bytes
        sendDataBlock(0);
        processedData += size;
    }
    else
    {
        // fast blocks
        byte blocks = size / HARDCODED_READ_DATABLOCK_SIZE;
        word actualTransferSize = blocks*HARDCODED_READ_DATABLOCK_SIZE;
        unsigned int blockNr = 0;
        //DBERR("create fast block of size: 0x%02x\n", actualTransferSize);

        // queue datablocks in reverse order
        for(int i=0; i<blocks; i++)
        {        
            //DBERR("newDataBlock[%u] addr: 0x%04x, offset: 0x%04x, size: %u\n", i, address, offset, READ_DATABLOCK_SIZE);
            dataBlockQueue.push_front(new DataBlock(blockNr, buffer, offset, address, HARDCODED_READ_DATABLOCK_SIZE));
            address += HARDCODED_READ_DATABLOCK_SIZE;
            offset += HARDCODED_READ_DATABLOCK_SIZE;
            blockNr++;
        }

        nwhSupport->sendHeader();
        nwhSupport->send(BLOCKREAD_FASTTRANSFER);
        nwhSupport->send16(startAddress+actualTransferSize);
        nwhSupport->send(blocks);
        for (unsigned int i=0; i<blocks; i++)
        {
            sendDataBlock(i);
        }
        processedData += actualTransferSize;        // store for use in blockReadContinue()        
   }
}

void BlockRead::sendDataBlock(unsigned int blocknr)
{
    DataBlock* dataBlock = dataBlockQueue[blocknr];
    //DBERR("sendDatablock[%d]: header: 0x%02x, transferAddress: 0x%04x\n", dataBlock->number, dataBlock->header, dataBlock->transferAddress);
/*
    static int error255 = 0;
    if (error255 == 0)
    {
        nwhSupport->send(0xff); // insert extra 0xff to simulate buffer underrun
        nwhSupport->send(0xff); // insert extra 0xff to simulate buffer underrun
        nwhSupport->send(0xff); // insert extra 0xff to simulate buffer underrun
        nwhSupport->send(0xff); // insert extra 0xff to simulate buffer underrun
    }
    error255++;
    if (error255 == 20) error255=0;
*/

    nwhSupport->send(dataBlock->header);    // header
    for (unsigned int i=0; i<dataBlock->data.size(); i++)
    {
        nwhSupport->send(dataBlock->data[i]);
        //DBERR("dataBlock[%i] -> data: 0x%02x\n", i, dataBlock->data[i]);
    }

    /*
    static int wrong = 0;
    if (wrong > 185)
    {
        DBERR("ERROR INSERTED for TESTing 4x 0xff added in data block\n");
        // this will cause BlockRead::ack to detect a wrong tail, and if all goes well, automatically resend the data.
        // todo: on a corrupt communication channel, this can cause infinate looping, the msx should timeout and the
        // host will automatically fall back to STATE_SYNC1 at the next valid command.
        nwhSupport->send(0xff); // insert extra 0xff to simulate buffer underrun
        nwhSupport->send(0xff); // insert extra 0xff to simulate buffer underrun
        nwhSupport->send(0xff); // insert extra 0xff to simulate buffer underrun
        nwhSupport->send(0xff); // insert extra 0xff to simulate buffer underrun
    }
    wrong++;
    DBERR("wrong: %i\n", wrong);
    //if (wrong == 20) wrong=0;
    */

    nwhSupport->send(dataBlock->header);    // tail
}

void BlockRead::blockReadContinue()
{
    unsigned int address = startAddress + processedData;
    if (processedData < transferSize)     // still bytes to be transferred?
    {
    /*
        DBERR("blockReadContinue, do more!\n");
        if (transferingToPage01) {
            DBERR("transferingToPage01 = TRUE!\n");
        } 
        else {
            DBERR("transferingToPage01 = FALSE!\n");
        }
    */
        if (transferingToPage01 && address >= TWOBANKLIMIT && processedData > 0) // address >= 0x8000 and this is not the first transfer?
        {
                //DBERR("send 3 -> continue at page 2/3\n");
                // switch to page23-transfers
                nwhSupport->sendHeader();
                nwhSupport->send(BLOCKREAD_EXIT_MORE_DATE_AHEAD); 
                transferingToPage01 = false;   
        }
        blockRead(address, transferSize-processedData);   // state is set to STATE_BLOCKREAD
    }
    else
    {
        //DBERR("blockReadContinue, we're done!\n");
        nwhSupport->sendHeader();
        nwhSupport->send(returnCode);
        done = true;
    }
}

void BlockRead::ack(byte tail)
{
    assert(dataBlockQueue.size() != 0);
    DataBlock* dataBlock = dataBlockQueue[0];
    //DBERR("ACK -> Datablock[%d]: header: 0x%02x, transferAddress: 0x%04x\n", dataBlock->number, dataBlock->header, dataBlock->transferAddress);

    if (dataBlock->header == tail)
    {		
		// block succesfully received
        delete dataBlock;
        dataBlock = 0;
        dataBlockQueue.pop_front();

		//DBERR("BlockRead::ack, tail matched, %d datablocks left\n", dataBlockQueue.size());
		if (dataBlockQueue.size() == 0)
        {
            blockReadContinue();
        }
    }
    else
    {
        static int errors = 0;
        errors++;
        
        // at >3.56Mhz failing ack's are normal (part of the protocol) 
        // letting some blocks fail and resend is still much faster then doing slower transfers.
        // even at 3.56Mhz block may fail, but it should be sporadically, otherwise something is
        // wrong or at least unusual (100% CPU load on the host side can cause that for example)
        
        DBERR("BlockRead::ack, block %u failed! (errors: %u, tail: 0x%02x, but received: 0x%02x)\n", dataBlock->number, errors, dataBlock->header, tail);
#ifdef WIN32
        general::beep(1750, 50);
#endif

	    nwhSupport->sendHeader();
	    if (dataBlock->fastTransfer)
	    {
	        nwhSupport->send(BLOCKREAD_FASTTRANSFER);			// not done, retry fast block follows
            nwhSupport->send16(dataBlock->transferAddress);
            nwhSupport->send(1);
        }
        else
        {
	        nwhSupport->send(BLOCKREAD_SLOWTRANSFER);			// not done, retry slow block follows
            nwhSupport->send16(dataBlock->transferAddress);
            nwhSupport->send16(dataBlock->size);
        }        
        sendDataBlock(0); // resend dataBlock
        dataBlockQueue.pop_front();
        dataBlockQueue.push_back(dataBlock);    // put dataBlock at end of queue
    }
}

void BlockRead::cancelWithCode(byte returnCode)
{
    nwhSupport->sendHeader();
    nwhSupport->send(returnCode);
    done = true;
}

} // namespace nwhost
