
static const byte HARDCODED_READ_DATABLOCK_SIZE = 128;	// hardcoded in blockRead (common.asm)

// dummy command (reads first 16Kb of disk as test)
void NowindHost::blockReadCmd()
{
    SectorMedium* disk = drives[0]->getSectorMedium();
    
    vector<byte> data(16*1024);
	if (disk->readSectors(&data[0], 0, 32)) {
		DBERR("readSectors error reading sector 0-31\n");
	}
    blockRead(0x8000, 0x4000, data);
}

void NowindHost::blockRead(word startAddress, word size, const vector <byte >& data)
{
    DBERR("blockRead, startAddress: 0x%04X, size: 0x%04X\n", startAddress, size);

    static const word TWOBANKLIMIT = 0x8000;
    if (startAddress < TWOBANKLIMIT)
    {
        word endAddress = std::min(TWOBANKLIMIT, startAddress + size);
        word transferSize = endAddress - startAddress;
        blockReadHelper(startAddress, transferSize, data);
    }
    else
    {
        blockReadHelper(startAddress, size, data);
    }  
    DBERR("blockRead, transferred: 0x%04X\n", transferred);
}

void NowindHost::blockReadHelper(word startAddress, word size, const vector <byte >& data)
{
    DBERR("blockRead() blockReadHelper: 0x%04x, size: 0x%02x, transferred: 0x%02x\n", startAddress, size, transferred);
    word address = startAddress;
    unsigned int offset = transferred;      //transferred is an offset within 'const vector <byte >& data'
    unsigned int blockNr = 0;
    
    // delete any blocks still in the dataBlockQueue (unacknowlged by msx, could be caused by timeouts)
    for(unsigned int i=0; i< dataBlockQueue.size(); i++)
    {        
        delete dataBlockQueue[i];
    }
    dataBlockQueue.clear();

    if (size < HARDCODED_READ_DATABLOCK_SIZE)
    {   
        // just 1 slow block
        dataBlockQueue.push_front(new DataBlock(0, data, offset, address, size));
        sendHeader();
        send(2);        // slow tranfer
        send16(startAddress);
        send16(size);   // amount of bytes
        sendDataBlock(0);
    }
    else
    {
        // fast blocks
        word address = startAddress;
        unsigned int offset = transferred;
        byte blocks = size / HARDCODED_READ_DATABLOCK_SIZE;
        unsigned int blockNr = 0;

        // queue datablocks in reverse order
        for(int i=0; i<blocks; i++)
        {        
            //DBERR("newDataBlock[%u] addr: 0x%04x, offset: 0x%04x, size: %u\n", i, address, offset, READ_DATABLOCK_SIZE);
            dataBlockQueue.push_front(new DataBlock(blockNr, data, offset, address, HARDCODED_READ_DATABLOCK_SIZE));
            address += HARDCODED_READ_DATABLOCK_SIZE;
            offset += HARDCODED_READ_DATABLOCK_SIZE;
            blockNr++;
        }

        sendHeader();
        send(1);            // fast transfer
        send16(startAddress+size);
        send(blocks);
        for (unsigned int i=0; i<blocks; i++)
        {
            sendDataBlock(i);
        }
        state = STATE_BLOCKREAD_ACK;
        transferred += size;        // store for use in blockReadContinue()        
   }
}

void NowindHost::sendDataBlock(unsigned int blocknr)
{
    DataBlock* dataBlock = dataBlockQueue[blocknr];
    DBERR("sendDatablock[%d]: header: 0x%02x, transferAddress: 0x%04x\n", dataBlock->number, dataBlock->header, dataBlock->transferAddress);

    send(dataBlock->header);    // header
    for (unsigned int i=0; i<dataBlock->data.size(); i++)
    {
        send(dataBlock->data[i]);
        //DBERR("dataBlock[%i] -> data: 0x%02x\n", i, dataBlock->data[i]);
    }
/*
    static int wrong = 0;
    if (wrong == 0)
    {
        send(0xff); // insert extra 0xff to simulate buffer underrun
        send(0xff); // insert extra 0xff to simulate buffer underrun
        send(0xff); // insert extra 0xff to simulate buffer underrun
        send(0xff); // insert extra 0xff to simulate buffer underrun
    }
    wrong++;
    if (wrong == 50) wrong=0;
*/
    send(dataBlock->header);    // tail
}

void NowindHost::blockReadContinue()
{
    unsigned sectorAmount = getSectorAmount();
    unsigned int size = sectorAmount * 512;
    unsigned address = getCurrentAddress();

    DBERR("blockReadContinue(), size: 0x%04x, address: 0x%04x, transferred: 0x%04x\n", size, address, transferred);
    
    // we should determine here, where to send 0, 1, 2 or 3!!!
    
    if (transferred < size)
    {
        DBERR("blockReadContinue, do more!\n");
        sendHeader();
        send(3);    
        blockRead(address, size-transferred, buffer);   // state is set to STATE_BLOCKREAD_ACK
    }
    else
    {
        DBERR("blockReadContinue, we're done!\n");
        sendHeader();
        send(0);
        state = STATE_SYNC1;
    }
}

void NowindHost::blockReadAck(byte tail)
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
        if (dataBlockQueue.size() == 0)
        {
            blockReadContinue();
        }
    }
    else
    {
        static int errors = 0;
        errors++;

        DBERR("block failed! (errors: %u)\n", errors);
	    sendHeader();
	    
	    if (dataBlock->fastTransfer)
	    {
	        send(0x01);			// not done, retry fast block follows
            send16(dataBlock->transferAddress);
            send(1);
        }
        else
        {
	        send(0x02);			// not done, retry slow block follows
            send16(dataBlock->transferAddress);
            send16(dataBlock->size);
        }        
        sendDataBlock(0); // resend dataBlock
        dataBlockQueue.pop_front();
        dataBlockQueue.push_back(dataBlock);    // put dataBlock at end of queue
    }
}

void NowindHost::blockWrite()
{

}
