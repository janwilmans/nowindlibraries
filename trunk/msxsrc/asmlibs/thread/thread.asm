		MODULE	Thread
		include "thread.inc" ; Thread defenitions and structs

;---------------------------------------------------------------------------------
;--- Thread interrupt routines
;---------------------------------------------------------------------------------
				
Init:
		DI
           	LD	HL,038h
             	LD	DE,oldInt
             	LD	BC,3
             	LDIR
             	LD	HL,IntHook
             	LD	DE,038h
             	LD	BC,3
             	LDIR
             	
             	; Store Int SP
             	LD	HL,0
             	ADD 	HL,SP
             	INC	HL
             	INC	HL	; sub ret adress from calling this function
             	LD	(threadEngine.oldSP),HL
        
        	; Default 1 thread is running (main)
             	LD	A,1
             	LD	(threadEngine.nrThreads),A 
             	LD	(threadEngine.threads + THREAD.ID),A  ; Thread ID
           	LD	(threadEngine.lastID),A  	
             	
             	;  Thread pointer for main thread
             	LD	HL,threadEngine.threads  
             	LD	(threadEngine.threadPtr),HL 
             	; The threads list is defragemented when threads are removed.
             	; An extra pointer is stored which is updated to point to the current
             	; position in the thread list (thread handle)
             	LD	(threadEngine.handles),HL
             	; Pointer to the handle
             	LD	HL,threadEngine.handles
             	LD	(threadEngine.threads + THREAD.handlePtr),HL
             	
             	LD	HL,threadEngine.idleThrdContext
             	LD	(threadEngine.idleThread.SP),HL
             	LD	HL,IdleThread
             	LD	(threadEngine.idleThrdContext.PC),HL
             	
             	EI 	
		RET

IntHook:	
		JP	IntHandler

Shutdown:
		DI
		LD	SP,(threadEngine.oldSP)
           	LD	HL,oldInt
             	LD	DE,038h
             	LD	BC,3
             	LDIR
		IN	A,(0AAh)
		SET	6,A
		OUT	(0AAh),A
		EI
		RET	

oldInt		DS	3			

		
		
IntHandler:
		DI
		; Store registers in the stack of the current thread
		PUSH	AF
		PUSH	BC
		PUSH	DE
		PUSH	HL
		
		IN	A,(99h) ;  clear int
		
		; If the context switch is not called directly don't update sleeps
		LD	A,(threadEngine.notCalledFromInt)
		OR	A,A
		CALL	Z,UpdateSleeps	
			
		; If bdos is active don't do a tread switch
		LD	A,(threadEngine.bdosActive)
		OR	A,A
		JR	NZ,.skipThreadSwitch

		EXX
		EX	AF,AF
		PUSH	AF
		PUSH	BC
		PUSH	DE
		PUSH	HL
		PUSH	IX
		PUSH	IY
		
		; store current SP which belongs to the thread
		LD	HL,0
             	ADD 	HL,SP
		LD	DE,HL
		LD	HL,(threadEngine.threadPtr) 

		INC	HL ; Skip objectID
		
		LDI	(HL),DE	; Store SP in thread struct
		; Pointer is now on state of a thread struct
		
		; Turn on caps light
		IN	A,(0AAh)
		RES	6,A
		OUT	(0AAh),A
		
.getThread	
		XOR	A,A
		LD	(threadEngine.notCalledFromInt),A	
		LD	BC,(threadEngine.currentThread); B = nrThreads, C = currentThread		
.loop		
		LD	DE,THREAD
		ADD	HL,DE
		INC	C	; Update current Thread		
		; Check if max is reached
		LD	A,(threadEngine.nrThreads)
		CP	A,C	; Index equal to number of threads?
		JP	NZ,.getThreadState
		LD	C,0	
		LD	HL,threadEngine.threads + THREAD.state	; Begin of thread list
.getThreadState		
		LD	A,(HL)
		OR	A,A
 		JP	NZ,.next		; Is the thread active? 0 is active, other is idle,busy_waiting,suspended
 		
 		LD	A,C
 		LD	(threadEngine.currentThread),A
 		; Get start of thread. (3 bytes back)
 		DEC	HL		; SPL 
 		DEC	HL		; SPH
 		DEC	HL		; objectID 
 		LD	(threadEngine.threadPtr),HL
 		INC	HL		;  HL points to location SP in thread struct

.getSP 		
 		LD	E,(HL)
		INC	HL
		LD	D,(HL)	; Get SP from struct
		EX	DE,HL
		LD	SP,HL
		JP	.restoreContext
.next		 
		DJNZ	.loop
		; Nothing to schedule
		; Schedule idle process
		LD	HL,threadEngine.idleThread
		LD	(threadEngine.threadPtr),HL
		LD	A,-1
		LD	(threadEngine.currentThread),A	
		INC	HL	; Skip object ID	
		JP	.getSP 		

.restoreContext		
		POP	IY
		POP	IX
		POP	HL
		POP	DE
		POP	BC
		POP	AF
		EX	AF,AF
		EXX
.skipThreadSwitch			
		POP	HL
		POP	DE
		POP	BC
		POP	AF				
		EI	
		RET		; return to thread


IdleThread:
; Don't push or pop in the idle thread, stack of idle 
; thread only has space for 24 values (Z80_CONTEXT)	
		
		DI
		IN	A,(0AAh)
		SET	6,A
		OUT	(0AAh),A
		EI		 
		HALT 
		JP	IdleThread

		
UpdateSleeps:
; Modifies AF,B,DE,HL

		LD	HL,threadEngine.threads + THREAD.state
		LD	A,(threadEngine.nrThreads)
		LD	B,A
.next
		LD	A,(HL)
		CP	A,STATE_IDLE
		JP	NZ,.noChange

		; Update sleep counter
		LD	DE,HL
		
		INC	HL	; Set to THREAD.SLEEP_TIMER
		DEC	(HL)
		JP	NZ,.skip
		INC	HL		
		DEC	(HL)
		JP	NZ,.skip
		XOR	A,A ; Sleep time passed, set state ACTIVE
.skip	
		LD	HL,DE	
		LD	(HL),A	
.noChange
		LD	DE,THREAD
		ADD	HL,DE
		DJNZ	.next
		RET
		
threadEngine	THREAD_ENGINE   ; Contains all thread paramaters
events 		EVENTS		; area for events, max events set by MAX_EVENTS

;---------------------------------------------------------------------------------
;--- Internal functions
;---------------------------------------------------------------------------------

		
EndThread:
		; Remove thread from threads struct
		DI	
		LD	DE,(threadEngine.threadPtr) ; get context ptr from the thread list		 
		LD	A,(threadEngine.currentThread)
		CALL	RemoveThread
	
		LD	A,(threadEngine.currentThread)		
		DEC	A
		LD	(threadEngine.currentThread),A
		
		; Get current thread pointer	
		LD	HL,(threadEngine.threadPtr)	
		LD	DE,-THREAD
		ADD	HL,DE		; Calculate previous pointer. There is always one thread in the list (main)
		; select next thread from list to excecute
		; Sleeps are not updated
		INC	HL ; Skip object ID
		INC	HL ; Skip SP
		INC	HL
		JP 	IntHandler.getThread


RemoveThread:
; Input A = thread index of thread to remove
;       DE = pointer to thread struct in list
		
		; First notify all clients that thread is ended
		LD	HL,THREAD.closingEvent
		ADD	HL,DE
		PUSH	AF
		CALL	NotifyClients
		POP	AF

		LD	HL,THREAD	
		ADD	HL,DE		; Calculate offset to next thread entry
		LD	B,A
		LD	A,(threadEngine.nrThreads)	
		DEC	A	
		SUB	A,B		; Calculate number of entries to move in thread list

		PUSH	AF
		PUSH	BC
		JR	Z,.NoMove
		
		LD	B,A     ; "*8"
		XOR	A,A
		
		SRL	B	; shift 3 times left, so it's multiplied with 32
		RRA		
		SRL	B
		RRA
		SRL	B
		RRA		; * THREAD
								
		LD	C,A
		LDIR		; Move threads
		
.NoMove	
		POP	AF	; A =  thread index of thread to remove
		POP	BC      ; B = number of handles to modify
	
		LD	HL,threadEngine.handles
		ADD	A,A
		ADD_HL_A
		LD	(HL),0 ; Clear handle
		INC	HL
		LD	(HL),0
		INC	HL
		LD	A,B ; B = number of handles to modify
		OR	A,A
		JR	Z,.noModify	
		
		LD	BC,-THREAD
.nextHandle	
		; Modify thread handles	
		LDI	DE,(HL)
		INC	D
		DEC	D	; Check if handles is unused
		JR	Z,.nextHandle
			
		DEC	HL
		DEC	HL
		EX	DE,HL
		ADD	HL,BC
		EX	DE,HL
		LDI	(HL),DE
		
		DEC	A
		JR	NZ,.nextHandle
		
.noModify:		
		LD	A,(threadEngine.nrThreads)	
		DEC	A
		LD	(threadEngine.nrThreads),A
		RET

	


SetThreadState:
; In A = state
;   HL = handle  
		PUSH	HL
		DI
		LD	E,(HL) ; Get pointer to the thread in the thread list
		INC	HL
		LD	D,(HL)
		DEC	D
		INC	D	; check if zero
		JR	Z,.error
		EX	DE,HL
		INC	HL	; Skip object type
		INC	HL
		INC	HL	; Skip SP, pointer to THREAD.state
		LD	(HL),A	
.error	
		EI
		POP	HL
		RET
		
GetFreeHandle:
; Input  HL = handles list
;         B = number of total handles
; Output HL = free handle position	
;         Z = Found
;        NZ = Not found
		XOR	A,A
.loop		
		INC	HL	; Don't care about low byte
		CP	A,(HL)
		JR	Z,.found
		INC	HL
		DJNZ	.loop
		; Not found
		OR	A,A
		RET
.found		
		DEC	HL
		RET
			
;---------------------------------------------------------------------------------
;--- User functions
;---------------------------------------------------------------------------------

Create:
; Input
; A  = param8	;	8 bit parameter
; BC = param16  ;       16 bit parameter
; DE = Start address
; HL = Stack address
; Output
;  A = Thread ID
; HL = Thread Handle
		DI	
		PUSH	HL
		; Set endthread function on the stack
		DEC	HL
		LD	(HL),EndThread / 256
		DEC	HL	
		LD	(HL),EndThread and 255		
				
		; Set start address on the thread stack	
		DEC	HL
		LD	(HL),D
		DEC	HL
		LD	(HL),E  ; 
		DEC	HL
		
		LD	(HL),A  ; 8 bit param 
		DEC	HL
		LD	(HL),0  ; flags on zero
		DEC	HL

		LD	(HL),B  ; 16 bit param 
		DEC	HL
		LD	(HL),C  ;  
		DEC	HL
		POP	HL
		
		; default stack offset
		LD	DE,-Z80_CONTEXT	
		ADD	HL,DE
		EX	DE,HL
		
		LD	A,(threadEngine.nrThreads)
		LD	L,A
		LD	H,0
		ADD	HL,HL
		ADD	HL,HL
		ADD	HL,HL
		ADD	HL,HL
		ADD	HL,HL ; * THREAD

		LD	BC,threadEngine.threads
		ADD	HL,BC 		; Calculate offset in thread struct 	
		PUSH	HL
		LD	(HL),OBJECT_THREAD
		INC	HL		; skip objectID		
		LDI     (HL),DE		; Store thread start stack address in the threads table
		LD	(HL),STATE_ACTIVE
		
		LD	A,(threadEngine.nrThreads)
		INC	A
		LD	(threadEngine.nrThreads),A
		
		INC	HL	; Sleep counter
		INC	HL	; Sleep counter
		INC	HL	; ID
		LD	(HL),0	; Clear ID
		CALL	GetUniqueId
		LD	(HL),A
		LD	(threadEngine.lastID),A
		
		LD	HL,threadEngine.handles+2 ; First thread can't be terminated, so handle is never free
		LD	B,MAX_THREADS-1
		CALL	GetFreeHandle
		POP	DE
		LD	(HL),DE	; Fill handle with pointer to thread struct
		EX	DE,HL
		LD	BC,THREAD.handlePtr
		ADD	HL,BC
		LD	(HL),DE		; Set handle pointer in thread struct
		EX	DE,HL
		LD	A,(threadEngine.lastID)
.error
		EI
		RET	
		
GetUniqueId:
; Output 
; A = Unique ID
		PUSH	HL
		; Get the last set ID, this ID is the best quess for a free ID
		LD	A,(threadEngine.lastID)
		INC	A
		JR	NZ,.checkID
		INC	A	; ID can't be zero
		LD	DE,THREAD
.checkID	
		LD	BC,(threadEngine.nrThreads-1)	; B = number of threads
		LD	HL,threadEngine.threads + THREAD.ID
.loop
		CP	A,(HL)
		JP	Z,.idInUse
		ADD	HL,DE	; Next
		DJNZ	.loop	
		; Unique ID found
		POP	HL
		RET		
		
.idInUse	
		INC	A
		JR	NZ,.checkID
		INC	A	; ID can't be zero
		JP	.checkID

		
Sleep:		
; Input DE = Sleep time in intterupts
; Modifies A
		PUSH	HL
		PUSH	DE
		DEC 	DE 	;Number of loops is in DE
		LD	A,D
		OR	A,E	; if time is zero skip it
		JR	Z,.skip
		
		INC	E
		INC	D
		
		DI
		LD	HL,(threadEngine.threadPtr)
		INC	HL ; Object ID
		INC	HL
		INC	HL ; SP
		LD	(HL),STATE_IDLE
		INC	HL
		LD	(HL),E
		INC	HL
		LD	(HL),D
		LD	A,1
		LD	(threadEngine.notCalledFromInt),A
		
		CALL	IntHandler	; Switch context
.skip		
		POP	DE
		POP	HL
		RET
		

Suspend:
; Input
; HL = Thread handle
		LD	A,STATE_SUSPENDED
		JP	SetThreadState
		
Resume:
; Input	
; HL = Thread handle
	
		XOR	A,A ; STATE_ACTIVE
		JP	SetThreadState		

Terminate:
; Input
; HL = Thread handle
		PUSH	HL
		DI
		LD	E,(HL)	 ; Get pointer to the thread in the thread list
		INC	HL
		LD	D,(HL)
		DEC	D
		INC	D
		JR	Z,.error
		PUSH	DE   
		LD	HL,threadEngine.threads
		EX	DE,HL
		OR	A,A
		SBC	HL,DE 	; Calculate index of thread to terminate
		
		SRL	HL
		SRL	HL
		SRL	HL
		SRL	HL
		SRL	HL	; / THREAD
		
		LD	A,L	; Thread index
		POP	DE	; Pointer to thread struct
		CALL	RemoveThread
.error		
		POP	HL
		EI	
		RET
		
GetId:
; Input
; HL = Thread handle
; Output 
; A  = Thread ID
		PUSH	HL
		DI
		LD	E,(HL)	 ; Get pointer to the thread in the thread list
		INC	HL
		LD	D,(HL)
		DEC	D
		INC	D
		JR	Z,.error
		EX	DE,HL
		LD	DE,THREAD.ID
		ADD	HL,DE
		LD	A,(HL)	
.error	
		EI
		POP	HL
		RET

Open:
; Input 
; A = Thread ID
; Output
;    Z = Found, NZ = Not found
; HL = handle
		PUSH	AF
		DI
		LD	HL,threadEngine.threads+THREAD.ID
		LD	A,(threadEngine.nrThreads)
		LD	B,A
		LD	DE,THREAD
.loop		
		CP	A,(HL)
		JP	Z,.found
		ADD	HL,DE	; Next thread	
		DJNZ	.loop
		; Not found, Zero flag cleared
		LD	A,-1
		OR	A,A
		EI
		POP	AF
		RET
.found
;		Zero flag set
		INC	HL
		LD	E,(HL)	; Get handle ptr
		INC	HL
		LD	D,(HL)
		EX	DE,HL
		EI
		POP	AF
		RET
					

GetCurrentHandle:
; Returns current thread handle in HL
		DI
		LD	IX,(threadEngine.threadPtr)
		LD	HL,(IX + THREAD.handlePtr)
		EI
		RET	

SwitchTo:
		DI
		LD	A,1
		LD	(threadEngine.notCalledFromInt),A
		JP	IntHandler	; Switch context
		 
		
				
MutexedBdosCall:
; C = bdoscall
		; Set bdos used flag
		PUSH	AF
		DI
		LD	A,1
		LD	(threadEngine.bdosActive),A			
		EI
		POP	AF
		
		CALL	BDOS		
	
		; Clear flag
		PUSH	AF
		XOR	A,A
		LD	(threadEngine.bdosActive),A
		POP	AF 
		RET


PrintThreads:
; HL = pointer to buffer
		DI
		LD	A,(threadEngine.nrThreads)
		LD	C,A
		LD	DE,threadEngine.threads
		LD	(HL),CR
		INC	HL
		LD	(HL),LF
		INC	HL
.loop	
		LD	IXL,8
.next	
		LD	A,(DE)
		INC	DE
		CALL	String.ByteToHex
		DEC	IXL
		JP	NZ,.next
		LD	(HL),CR
		INC	HL
		LD	(HL),LF
		INC	HL
		DEC	C
		JP	NZ,.loop
		
		LD	DE,threadEngine.handles		
		LD	C,10
.next2		
		LD	A,(DE)
		INC	DE
		CALL	String.ByteToHex
		LD	A,(DE)
		INC	DE
		CALL	String.ByteToHex
		LD	(HL),' '
		INC	HL
		DEC	C
		JP	NZ,.next2
		
		LD	(HL),CR
		INC	HL
		LD	(HL),LF
		INC	HL
		LD	(HL),LE		
		EI
		RET

NotifyClients:
; HL = Pointer to EVENT struct
		INC	HL     ; Skip objectID
		LD	A,1
		LD	(HL),A	; Set signaled
		INC	HL
		
		LD	A,(HL) ; Number of waiting threads
		OR	A,A
		RET	Z      ; No waiting threads
		LD	(HL),0 ; Clear nr of waiting threads
		INC	HL
		LD	B,A
		PUSH	DE
.loop		
		LDI	DE,(HL)	; Get Handle from list
		PUSH	HL
		EX	DE,HL
		; Get thread struct pointer
		LD	E,(HL)
		INC	HL
		LD	D,(HL)
		EX	DE,HL
		INC	HL ; Skip object ID
		INC	HL
		INC	HL ; Skip SP
		LD	(HL),STATE_ACTIVE
		POP	HL
		DJNZ	.loop
		POP	DE	
		RET

CloseHandle:
		DI
		LD	E,(HL)
		INC	HL
		LD	D,(HL)
		DEC	D
		INC	D
		JR	Z,.invalidHandle
		EX	DE,HL
		; Clear object pointer, which can be a thread or event
		LD	(HL),0
		INC	HL
		LD	(HL),0
		EI
.invalidHandle
		RET

CreateEvent:
		DI
		LD	HL,events.handles
		LD	B,MAX_EVENTS
		CALL	GetFreeHandle
		JR	NZ,.error
		; Found free handle
		PUSH	HL ; Handle
		LD	A,MAX_EVENTS
		SUB	A,B		; Calc event index
		; Calc struct offset
		LD	L,A
		LD	H,0
		ADD	HL,HL
		ADD	HL,HL
		ADD	HL,HL ; * 8
		LD	DE,HL
		ADD	HL,HL ;* 16		
		ADD	HL,DE ;* 24
		LD	E,A
		LD	D,0
		SBC	HL,DE;*23  
		LD	DE,events.events
		ADD	HL,DE
		EX	DE,HL
		POP	HL ; handle
		PUSH	HL ; handle
		LD	(HL),DE ; Store event pointer struct in event handle list
		EX	DE,HL
		LD	(HL),OBJECT_EVENT

		; Clear event struct
		INC	HL
		LD	DE,HL
		INC	DE
		LD	(HL),0
 		LD	BC,EVENT-2  ; minus to because objectID doesn't need to be cleared
		LDIR
		
		POP	HL ; Handle
		LD	A,1
		OR	A,A
.error		
		EI
		RET	
SetEvent:
; Input HL = event handle
		DI
		; Get thread pointer
		LD	E,(HL)
		INC	HL
		LD	D,(HL)
		DEC	D
		INC	D
		JR	Z,.invalidHandle
		EX	DE,HL
		LD	A,(HL)	; object type
		CP	A,OBJECT_EVENT
		JR	NZ,.invalidHandle
		CALL	NotifyClients
.invalidHandle		
		EI
		RET
		
WaitForSingleObject:
; Input HL = Handle
		DI
		; Get thread pointer
		LD	E,(HL)
		INC	HL
		LD	D,(HL)
		DEC	D
		INC	D
		JR	Z,.invalidHandle
		
		; Check object type
		LD	A,(DE)
		OR	A,A	; OBJECT_THREAD
		JR	NZ,.event
		
		; Get number of registered from thread events
		LD	HL,THREAD.closingEvent.signaled
		ADD	HL,DE		; DE = thread pointer
		JP	.checkSignaled	
.event		
		; Object is an event
		EX	DE,HL
		INC	HL	; skip objectId
.checkSignaled		
		LD	A,(HL)	; Get signaled state
		OR	A,A	; Signaled?
		LD	(HL),0	; Clear flag
		JP	NZ,.exit
		INC	HL	
.addHandle		
		LD	A,(HL)		; Get number of events in list
		INC	(HL)		; handles++
		INC	HL		; event.handles
		ADD	A,A		
		ADD_HL_A		; Pointer to empty event list entry
		
		LD	IX,(threadEngine.threadPtr)
		LD	(IX + THREAD.state),STATE_BUSY_WAITING
		
		; Store handle in Event handles list
		LD	DE,(IX + THREAD.handlePtr)
		LD	(HL),E
		INC	HL
		LD	(HL),D
	
		LD	A,1
		LD	(threadEngine.notCalledFromInt),A
		CALL	IntHandler	; Switch context
.invalidHandle
.exit
		EI
		RET

AllocateStackSpace:
; Input
; 		BC = Allocate size
; Output :	HL = Pointer to allocated stack space
;	
; Mofifies BC,HL,IX
		POP	IX			; Get return address
		LD	HL,0
		ADD	HL,SP
		OR	A,A
		SBC	HL,BC
		LD	SP,HL
		PUSH	BC			; Store object size on stack
		LD	BC,FreeStackSpace
		PUSH 	BC			; Free stack function on stack
		JP	(IX)
		
FreeStackSpace:
		POP	HL	; Get object size
		ADD	HL,SP
		LD	SP,HL	; Restore Stack
		RET
		
				
		ENDMODULE
		
				
			