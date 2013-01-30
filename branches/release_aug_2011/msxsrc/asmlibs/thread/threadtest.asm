		define G9K_DISABLE_PATTERN
		define G9K_DISABLE_VFF
		
		include "..\macros.inc"
		include "..\bdos\bdos.inc"
		include "..\keyboard\keyboard.inc"
		include "..\G9klib\g9klib.inc"
			
		OUTPUT  "bin\thrdtst.com"
		
		org 	0100h

Main:		
		; Initialize and start thread engine
		; Current program and stack added to the schedular
		CALL	Thread.Init

		CALL	Thread.CreateEvent
		LD	(hEvent),HL
			
		LD	DE,Thread1	; Start address
		LD	HL,0A800h	; Stack address
		LD	A,1		; thread parameter
		LD	BC,2		; Var timer
		CALL	Thread.Create	; Create and run thread
		PUSH	AF	
		LD	(hThread1),HL	; Store thread handle
		LD	(threadID1),A   ; Store thread ID
		CALL	Thread.Suspend
		POP	AF
		CALL	PrintThreadCreated ; Print handle and thread ID
		
		LD	DE,Thread1
		LD	HL,0B000h	; Stack address
		LD	A,2		; thread parameter
		LD	BC,20		; Var timer
		CALL	Thread.Create
		CALL	PrintThreadCreated

		LD	DE,Thread2
		LD	HL,0B800h
		CALL	Thread.Create
		CALL	PrintThreadCreated

		LD	DE,G9BViewThread
		LD	HL,0C000h
		CALL	Thread.Create
		CALL	PrintThreadCreated
		
		CALL	Thread.GetCurrentHandle
		LD	(hMainThread),HL
		
		; Main thread waits until event is set by thread2
		LD	HL,(hEvent)
		CALL	Thread.WaitForSingleObject

.waitKey
		LD	DE,5
		CALL	Thread.Sleep
		
		LD	C,8
		CALL	Keyboard.GetDirectKey
		BIT	KEY_SPACE_BAR,A
		JP	NZ,.waitKey	
		
		LD	HL,(hThread1)
		CALL	Thread.Terminate
		LD	A,(threadID1)
		CALL	PrintThreadTerminated
		
		LD	DE,60
		CALL	Thread.Sleep
		
.scanLoop
; Check every 5/60 seconds if space bar is pressed
		LD	DE,5
		CALL	Thread.Sleep

		LD	C,8
		CALL	Keyboard.GetDirectKey
		BIT	KEY_SPACE_BAR,A
		JP	NZ,.scanLoop	
.exit:
;  Shutdown all threads and resume 'normal' environment
		CALL	Thread.Shutdown
		RET		
	
		
hMainThread     DW	0               
hThread1	DW	0
threadID1	DB	0
hEvent		DW	0
              
Thread1:
		LD	DE,BC	; parameter used for sleep
		ADD	A,'0'
.main
		PUSH	AF
		PUSH	DE
		LD	C,_CONOUT
		LD	E,A
		CALL	Thread.MutexedBdosCall
		POP	DE
		CALL	Thread.Sleep
		POP	AF
		
		JP	.main
.end		
		RET	

Thread5:
		PUSH	AF
		PUSH	BC
		; Wait for thread1 to close
		LD	HL,(hThread1)
		CALL	Thread.WaitForSingleObject
		POP	DE	; parameter used for sleep
		POP	AF
		ADD	A,'0'
.main
		PUSH	AF
		PUSH	DE
		LD	C,_CONOUT
		LD	E,A
		CALL	Thread.MutexedBdosCall
		POP	DE
		CALL	Thread.Sleep
		POP	AF
		
		JP	.main
.end	
	
		RET	


Thread2:

.main
		; Wait
		LD	DE,60*10
		CALL	Thread.Sleep
		
		LD	HL,(hEvent)
		CALL	Thread.SetEvent
		
		LD	HL,(hThread1)
		CALL	Thread.Resume
		
		LD	DE,Thread1
		LD	HL,09800h
		LD	A,3		; thread parameter
		LD	BC,60		; 1 time per second
		CALL	Thread.Create
		CALL	PrintThreadCreated
		
		LD	DE,Thread5
		LD	HL,0A000h
		LD	A,4		; thread parameter
		LD	BC,120		; 1 time per 2 seconds
		CALL	Thread.Create
		CALL	PrintThreadCreated
				
		CALL	Thread.GetCurrentHandle
		CALL	Thread.GetId
		CALL	PrintThreadExited
		RET	
		
G9BViewThread:
; This threads displays a series of pictures on a Gfx9000 video card
; If it's not found the thread stops
		CALL	G9k.Detect	; Gfx9000 not found
		JR	NZ,.exit
		CALL	G9k.Reset
		
		; Set default blitter registers
		G9kWriteReg G9K_ARG,0
		G9kWriteReg G9K_LOP,G9K_LOP_WCSC
		LD      HL,#FFFF
		CALL    G9k.SetCmdWriteMask
		
		; Set display mode
		LD	A,G9K_MODE_B3
		LD	BC,G9K_SCR0_8BIT * 256 + G9K_SCR0_XIM512
		LD	DE,1 * 256 + G9K_PAL_CTRL_YUV
		CALL	G9k.SetScreenMode
		CALL	G9k.DisplayEnable
		
		LD	DE,fileName2
		CALL	LoadG9B
		JR	NZ,.exit
		
		LD	DE,60*5	; Wait 5 secondes
		CALL	Thread.Sleep

		LD	DE,fileName
		CALL	LoadG9B
		JR	NZ,.exit
		
		LD	DE,60*5	; Wait 5 secondes
		CALL	Thread.Sleep
		
		LD	DE,fileName3
		CALL	LoadG9B
.exit	
		CALL	Thread.GetCurrentHandle
		CALL	Thread.GetId
		CALL	PrintThreadExited
		RET
		
LoadG9B:
		LD	BC,G9B_OBJECT
		CALL	Thread.AllocateStackSpace
		; HL = pointer to object space on stack

		PUSH	DE
		PUSH	HL
		LD	C,_STROUT
		CALL	Thread.MutexedBdosCall		
		POP	HL
		POP	DE
			
		PUSH	HL
		CALL	G9k.OpenG9B
		POP	IX
		RET	NZ
		
		LD	HL,0		    ; HL = Destination X
		LD	IY,0		    ; IY = Destination Y
		LD	DE,BUFFER
	        LD	BC,16384	    ; Buffer size
	        LD	A,0	      	    ; Palette pointer
		CALL	G9k.ReadG9B		
		JP	G9k.Close
		
fileName:	DB	"picture1.g9b",0,CR,LF,LE
fileName2:	DB	"picture2.g9b",0,CR,LF,LE
fileName3:	DB	"picture3.g9b",0,CR,LF,LE

PrintThreadCreated:
; Input A = Thread ID
;      HL = Thread handle		
		
		CALL	PrintInfo
		LD	DE,createdTxt
		LD	C,_STROUT
		JP	Thread.MutexedBdosCall	
		
PrintThreadTerminated:
; Input A = Thread ID
;      HL = Thread handle		
		
		CALL	PrintInfo
		LD	DE,terminatedTxt
.print	
		LD	C,_STROUT
		CALL	Thread.MutexedBdosCall	
		
		LD	DE,threadTxt
		LD	C,_STROUT
		JP	Thread.MutexedBdosCall	

PrintThreadExited:
		
		CALL	PrintInfo
		LD	DE,exitedTxt
		JP	PrintThreadTerminated.print	
		
		
PrintInfo:		
		PUSH	HL
		LD	HL,idTxt
		CALL	String.ByteToHex
		POP	HL
		LD	A,H
		LD	C,L
		LD	HL,handleTxt
		CALL	String.ByteToHex
		LD	A,C
		JP	String.ByteToHex

		
createdTxt	DB	CR,LF,"Created"
threadTxt	DB	"Thread - ID 0x"
idTxt		DB	"  "
		DB	" Handle - 0x"
handleTxt		DB	"     ",LE

terminatedTxt	DB	CR,LF,"Terminated ",LE 
exitedTxt	DB	CR,LF,"Exited ",LE


		include "thread.asm"			; Thread library
		include "..\keyboard\keyboard.asm"	; Keyboard library
		include "..\string\string.asm" 		; String manipulation library
		include "..\G9klib\g9klib.asm"		; Graphics library for Gfx9000 video card
		include "file.asm"			; File functions library
		include "..\bitbuster\bitbuster.asm"	; Graphics decompressing library used by g9klib.asm
		include "..\math\math.asm"		; 
	
		
TXT_BUFFER:	DS	512		
BUFFER:	
	 
	 
	 