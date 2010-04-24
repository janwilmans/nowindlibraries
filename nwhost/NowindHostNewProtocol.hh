
static const byte HARDCODED_READ_DATABLOCK_SIZE = 128;	// hardcoded in blockRead (common.asm)

void NowindHost::newBlockTransfer(unsigned transferAddress, unsigned amount)
{
    //DBERR("create new BlockTranfer: addr: 0x%04x  amount %d\n", transferAddress, amount);
    vector<byte> temp;
	const byte* bufferPointer = &buffer[transferred];
	for (unsigned int i=0;i<amount; i++) {
		temp.push_back(bufferPointer[i]);
	}
    blockRead(transferAddress, amount, temp);
}

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
    //DBERR("blockRead() startAddress: 0x%04x, size: 0x%02x\n", startAddress, size);

    for(unsigned int i=0; i< dataBlockQueue.size(); i++)
    {        
        delete dataBlockQueue[i];
    }
    dataBlockQueue.clear();

    word address = startAddress;
    unsigned int offset = 0;
    byte blocks = size / HARDCODED_READ_DATABLOCK_SIZE;

    // queue datablocks in reverse order
    for(int i=0; i<blocks; i++)
    {        
        //DBERR("newDataBlock[%u] addr: 0x%04x, offset: 0x%04x, size: %u\n", i, address, offset, READ_DATABLOCK_SIZE);
        dataBlockQueue.push_front(new DataBlock(i, data, offset, address, HARDCODED_READ_DATABLOCK_SIZE));
        address += HARDCODED_READ_DATABLOCK_SIZE;
        offset += HARDCODED_READ_DATABLOCK_SIZE;
    }

    sendHeader();
    send(1);            // not end 
    send16(startAddress+size);
    send(blocks);
    for (unsigned int i=0; i<blocks; i++)
    {
        sendDataBlock(i);
    }
    state = STATE_BLOCKREAD;
}

void NowindHost::sendDataBlock(unsigned int blocknr)
{
    DataBlock* dataBlock = dataBlockQueue[blocknr];
    //DBERR("sendDatablock[%d]: header: 0x%02x, transferAddress: 0x%04x\n", dataBlock->number, dataBlock->header, dataBlock->transferAddress);

    send(dataBlock->header);    // header
    for (unsigned int i=0; i<dataBlock->data.size(); i++)
    {
        send(dataBlock->data[i]);
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
    }
    else
    {
        static int errors = 0;
        errors++;

        DBERR("block failed! (errors: %u)\n", errors);
	    sendHeader();
	    send(0x01);			// not done, retry block follows
        send16(dataBlock->transferAddress);
        send(1);
        sendDataBlock(0); // resend dataBlock
        dataBlockQueue.pop_front();
        dataBlockQueue.push_back(dataBlock);    // put dataBlock at end of queue
    }

    if (dataBlockQueue.size() == 0)
    {
        // all blocks ack'd, we're done
        sendHeader();   // TODO: remove, MSX should check if errors occured
        send(0);
        state = STATE_SYNC1;
    }
}

void NowindHost::blockWrite()
{

}
