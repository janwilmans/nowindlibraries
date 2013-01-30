MAX_THREADS		EQU	10	; Maximum thread number,including 'main'
MAX_EVENTS		EQU     4       ; Maximum events

; Thread states
STATE_ACTIVE		EQU	0
STATE_IDLE		EQU	1
STATE_SUSPENDED 	EQU	2
STATE_BUSY_WAITING	EQU	3

; Object types
OBJECT_THREAD		EQU	0
OBJECT_EVENT		EQU	1

; Event struct can handle 10 waiting threads. Why 10?, because
; a thread struct is than 32 bytes in total.

			STRUCT EVENT ; size = 23
objectID:               BYTE
signaled:               BYTE		 ; Indicates if an event is signaled, auto reset by waiting thread
nrHandles:		BYTE		 ; Nr of waiting threads
handles:		BLOCK	10*2	 ; Fixed number of waiting events
			ENDS

			STRUCT EVENTS
nrEvents:		BYTE
handles:		BLOCK	MAX_EVENTS*2	
events:			BLOCK	EVENT*MAX_EVENTS			
			ENDS
			
			
; If the size is changed then some functions need to modified to 
; calculate offsets in the thread table  (RemoveThread and Terminate)
			STRUCT THREAD   ; minus closing events 9 bytes in size
objectID:               BYTE		; Object ID	
SP:			WORD		; Stack pointer of thread
state:			BYTE            ; Current thread state
sleepCount:		WORD            ; Sleep counter for sleep
ID:			BYTE	        ; Thread ID
handlePtr:		WORD	        ; Pointer to thread handle
closingEvent:		EVENT           ; Closing events for threads waiting on thread to terminate
			ENDS		
		
			STRUCT Z80_CONTEXT
IY:			WORD	
IX:			WORD
HL1:			WORD
DE1:			WORD
BC1:			WORD
AF1:			WORD
HL0:			WORD
DE0:			WORD
BC0:			WORD
AF0:			WORD
PC:			WORD
endThread:		WORD
			ENDS		

			STRUCT Z80_CONTEXT_EXT
SLOT_ID			BYTE			
PAGE1			BYTE			
PAGE2			BYTE			
IY:			WORD	
IX:			WORD
HL1:			WORD
DE1:			WORD
BC1:			WORD
AF1:			WORD
HL0:			WORD
DE0:			WORD
BC0:			WORD
AF0:			WORD
PC:			WORD
endThread:		WORD
			ENDS	

; Don't change any position of the fields in this structure  
			STRUCT	THREAD_ENGINE
oldSP:			WORD		
bdosActive:		BYTE
notCalledFromInt:	BYTE
lastID:			BYTE
currentThread:		BYTE    ; Index of current thread   
nrThreads:		BYTE	; total number of treads 
threadPtr:		WORD	; pointer to current thread struct
idleThread		THREAD     
threads:		BLOCK THREAD*MAX_THREADS,0  
idleThrdContext:	Z80_CONTEXT	
handles:		BLOCK 2*MAX_THREADS,0  
			ENDS

		