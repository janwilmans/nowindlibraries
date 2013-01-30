DisableV99x8Int
		DI
		LD	A,(0F3E0h)   ; GET CONTENTS V99X8 REG 1
		RES	5,A          ; Disable int
		LD	(0F3E0h),A   
		OUT	(099H),A
            	LD	A,1+128
             	OUT	(099H),A
             	IN	A,(99h)
             	
             	LD	HL,038h
             	LD	DE,oldInt
             	LD	BC,3
             	LDIR
             	LD	HL,MyIntHook
             	LD	DE,038h
             	LD	BC,3
             	LDIR
             	EI
		RET
		
EnableV99x8Int
		DI
		LD	A,(0F3E0h)	; GET CONTENTS V99X8 REG 1
		SET	5,A
		LD	(0F3E0h),A   
		OUT	(099H),A
            	LD	A,1+128
             	OUT	(099H),A
             	LD	HL,oldInt
             	LD	DE,038h
             	LD	BC,3
             	LDIR
             	EI
		RET
		
MyIntHook	JP	MyIntHandler
		
MyIntHandler
		DI
		PUSH	AF
		EX	AF,AF
		PUSH	AF
		PUSH	BC
		PUSH	DE
		PUSH	HL
		EXX
		PUSH	BC
		PUSH	DE
		PUSH	HL
		PUSH	IX
		PUSH	IY
		CALL	0fd9Ah
		POP	IY
		POP	IX
		POP	HL
		POP	DE
		POP	BC
		EXX
		POP	HL
		POP	DE
		POP	BC
		POP	AF
		EX	AF,AF
		POP	AF
		EI
		RET

oldInt		DS	3	