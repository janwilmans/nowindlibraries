 		ifndef Window_ASM
		define Window_ASM
		
		include "window.inc" 
		 
		MODULE Window
	
Draw2ColorBox:
; IX = left
; IY = top
; HL = width
; BC = height
; Modifies af,de

		
		PUSH	IY
		PUSH	IX
		PUSH	HL
		PUSH	BC

		LD	(.param.left),IX
		LD	(.param.top),IY
		
		PUSH	HL ; Width
		
		; Draw upper line
		LD	(.param.width),HL ; MJ
		LD	HL,0
		LD	(.param.height),HL
		LD	DE,LEFT_UP_COLOR
		PUSH	BC  ; Height
		G9kCmdWait		
		G9kWriteReg G9K_ARG,0	
		LD	HL,.param
		CALL	G9k.DrawLine
		POP	BC ; Height
		
		; Draw left line
		PUSH	BC ; Height
		LD	(.param.width),BC   ;MJ
		LD	HL,0
		LD	(.param.height),HL  ;MI
		G9kCmdWait
		G9kWriteReg G9K_ARG,G9K_ARG_MAJ
		LD	HL,.param
		CALL	G9k.DrawLine
		POP	BC ; Height
		POP	DE ; Width
		
		; Draw right line
		PUSH	IX
		PUSH	DE ; Width
		PUSH	BC
		ADD	IX,DE ; left + with

		LD	(.param.left),IX
		LD	HL,.param
		LD	DE,RIGHT_DOWN_COLOR
		
		CALL	G9k.DrawLine
		POP	BC  ; Height
		POP	DE  ; Width
		POP	IX  ; Left
		
		
		; Draw bottom line
		LD	(.param.left),IX
		ADD	IY,BC ; top + Height
		LD	(.param.top),IY
		LD	(.param.width),DE   ;MJ
		LD	HL,0
		LD	(.param.height),HL  ; MI
		LD	HL,.param
		LD	DE,RIGHT_DOWN_COLOR
		G9kCmdWait
		G9kWriteReg G9K_ARG,0
		CALL	G9k.DrawLine
		
		POP	BC
		POP	HL	
		POP	IX
		POP	IY
		RET

.param:		G9K_BOX
		
DrawOutLine:
; IX = left
; IY = top
; HL = width
; BC = height	
		PUSH	IX
		PUSH	IY
		PUSH	HL
		PUSH	BC	
		CALL	Draw2ColorBox
		INC	IX
		INC	IY
		LD	(.param.left),IX
		LD	(.param.top),IY

		DEC	BC
		DEC	BC
		LD	(.param.height),BC
		DEC	HL
		DEC	HL
		
		LD	(.param.width),HL
		LD	HL,INNER_COLOR
		G9kCmdWait
		CALL	G9k.SetCmdColor	
		LD	HL,.param	
		CALL 	G9k.DrawBox
		POP	BC
		POP	HL
		POP	IY
		POP	IX
		RET
		
.param:		G9K_BOX

DrawInnerLine:
; IX = left
; IY = top
; HL = width
; BC = height
		PUSH	IX
		PUSH	IY
		PUSH	HL
		PUSH	BC

		LD	(.param.left),IX
		LD	(.param.top),IY
		LD	(.param.width),HL
		LD	(.param.height),BC
		
		PUSH	HL
		PUSH	BC
		LD 	HL,INNER_COLOR
		G9kCmdWait
		CALL	G9k.SetCmdColor	
		LD	HL,.param
		CALL	G9k.DrawBox
		POP	BC
		POP	HL	
		INC	IX ; left + 1 
		INC	IY ; top + 1
		DEC	BC ; height - 2
		DEC	BC
		DEC	HL ; width - 2
		DEC	HL
		CALL	Draw2ColorBox
		
		POP	BC
		POP	HL
		POP	IY
		POP	IX	
		RET
		
.param:		G9K_BOX

DrawWindow:
; IX = left
; IY = top
; HL = width
; BC = height	
		
		PUSH	IX
		PUSH	IY
		PUSH	HL
		PUSH	BC
	
		CALL	DrawOutLine
		LD	DE,2
		ADD	IX,DE ; x + 2
		ADD	IY,DE ; y + 2	
		LD	(.param.left),IX
		LD	(.param.top),IY
		LD	DE,-3
		ADD	HL,DE ; width-3
		LD	(.param.width),HL
		LD	H,B
		LD	L,C
		ADD	HL,DE ; height-3
		LD	(.param.height),HL
		LD	HL,.param
		LD	DE,LEFT_UP_COLOR
		CALL	G9k.DrawFilledBox
		
		POP	BC
		POP	HL
		POP	IY
		POP	IX	
		RET
		
.param:		G9K_BOX


SetObjectList:
; HL: address of object list

      	      	 LD	(GetObject.objectList),HL
      	      	 LD	HL,0
		 LD	(GetEvent.prevMouseOver),HL
      	      	 LD	(GetEvent.prevMouseOut),HL
      	      	 RET



GetObject:
; Returns HL, functions pointer
;         IX, pointer to window object

		 LD	IX,(.objectList)
		 LD	B,(IX)
		 INC	IX

.loop:           LD	HL,(CursorData.x)
		 LD	DE,(IX + WINDOW.left)
		 CALL	Math.CPHLDE
		 JR	C,.next

		 LD	DE,(IX + WINDOW.right)
		 EX	DE,HL
		 CALL	Math.CPHLDE
		 JR	C,.next

                 LD	HL,(CursorData.y)
		 LD	DE,(IX + WINDOW.top)
		 CALL	Math.CPHLDE
		 JR	C,.next

		 LD	DE,(IX + WINDOW.bottom)
		 EX	DE,HL
		 CALL	Math.CPHLDE
		 JR	C,.next
		
		 LD	HL,(IX + WINDOW.onMouseClick)
		 RET

.next:           LD	DE,WINDOW
		 ADD	IX,DE
		 DJNZ	.loop
		 LD	HL,0
		 ret
		 
.objectList:	 DW 	0


GetEvent:
		
.checkDown:	
		;  Check mouse over	
		CALL    Window.GetObject
		LD	DE,(.prevMouseOver)
		CALL	Math.CPHLDE
		JR	Z,.NotChanged
		
		; IF HL=0 then no mouse over
		LD	A,H
	        OR	A,L
	        JR	Z,.mouseOut
	     	LD	(.prevMouseOver),HL
	     	
	     	; If Mouse moves from on object to another then also call prevMouseOut
	     	LD	HL,(.prevMouseOut)
		CALL	CallHL
	     	
	     	; If HL!=0 call mouse over routine
	     	LD	HL,(IX + WINDOW.onMouseOut)	
	     	LD	(.prevMouseOut),HL  
	     	
		LD	HL,(IX + WINDOW.onMouseOver)	 ; // data field contains mouse over function   
		CALL	CallHL
		JR	.NotChanged
				
.mouseOut:		
		LD	(.prevMouseOver),HL
		LD	HL,(.prevMouseOut)  
		CALL	CallHL
		LD	HL,0
		LD	(.prevMouseOut),HL
		
.NotChanged:		
		LD	A,(CursorData.A)
		OR	A,A
		JR	Z,.checkDown
				
		CALL    Window.GetObject
		PUSH	HL
		
		
		; Set pressed state of button
		LD	A,H
		OR	A,L	; Any button found?
		LD	HL,(IX + WINDOW.onMousePressed)
		CALL	NZ,CallHL
		
.checkUp
		LD	A,(CursorData.A)
		OR	A,A
		JR	NZ,.checkUp
		
		
		CALL    Window.GetObject

		PUSH	HL		
		LD	A,H
		OR	A,L	; Any button found?
		LD	HL,(IX + WINDOW.onMouseReleased)
		CALL	NZ,CallHL
		POP	HL
		
		POP	DE
		CALL	Math.CPHLDE
	        JR	NZ,GetEvent

	        LD	A,H
	        OR	A,L
	        JP	Z,GetEvent
		JP	(HL)

.prevMouseOver  DW	0
.prevMouseOut   DW	0

CallHL:
		LD	A,H
		OR	A,L
		RET	Z
		PUSH	HL
		RET	

		ENDMODULE
		endif
		
		
		
