mapper		MAPPER		; Outside the module. There is only one instance needed of this 


		MODULE Mapper

Init:
		; Get mapper support routines
		XOR	A,A
		LD	DE,4*256+2
		CALL	EXT_BIO
		LD      (mapper.mapperSupport),HL
		LD	(mapper.nrSegments),A
		LD	A,B
		LD	(mapper.priMapperSlot),A
		LD	A,C
		LD	(mapper.freeSegments),A
		LD	DE,mapper.AllocateSegment
		LD	BC,16*3 ; size of jump table
		LDIR
		RET
		
		ENDMODULE