
		IFNDEF	CURSOR_ASM
		DEFINE	CURSOR_ASM
		
		
		include	"cursor.inc"           
	
; Installs cursor on hook
; In: HL: Width
;     DE: Height
;     BC: Bottom
;     IX: Right		
InstallCursor:	ld	(CursorData.Width),hl
	        ld	(CursorData.Height),de

		di
		
		G9kReadReg	G9K_INT_ENABLE+G9K_DIS_INC_READ
		or	G9K_INT_IEV
                out	(G9K_REG_DATA),a		;enable V9990 interrupt
                
                ld	a,(SLOT_ID_RAM_0)
                ld	(CursorHook+1),a
                
                ld	hl,$fd9a
                ld	de,CursorHandler.OldHook
                ld	bc,5
                ldir
                
                ld	hl,CursorHook
                ld	de,$fd9a
                ld	bc,5
                ldir
                
                ei
                ret
                
                
KillCursor:	di
		ld	hl,CursorHandler.OldHook
		ld	de,$fd9a
		ld	bc,5
		ldir
		
		G9kReadReg	G9K_INT_ENABLE+G9K_DIS_INC_READ
		and	255-G9K_INT_IEV
                out	(G9K_REG_DATA),a		;disable V9990 interrupt		
		
		ei
		ret	
		
		


CursorHook:	db	$f7
		db	0		;slotid
		dw	CursorHandler	;call
		ret
	
		 	
CursorHandler:	
		in	a,(G9K_INT_FLAG)
		rra
		jp	nc,.OldHook 
		
		ld	a,1
		out	(G9K_INT_FLAG),a

		call	Mouse.GetXY
		
		ld	a,h
		or	l
		jr	z,.no_mouse_mov		;no mouse movement

		ld	a,l
		add	a,a   
		add	a,a
		ld	(.move_data_mouse),a
		cp	128
		ld	a,0
		jr	c,.skip0
		ld	a,255
.skip0:		ld	(.move_data_mouse+1),a
		
		ld	a,h  
		add	a,a    
		add	a,a
		ld	(.move_data_mouse + 2),a
		cp	128
		ld	a,0     
		jr	c,.skip1
		ld	a,255
.skip1:		ld	(.move_data_mouse + 3),a
		
		call	Stick.GetStick
		ld	(CursorData.A),bc
		
		ld	hl,.move_data_mouse
		jr	.move

.no_mouse_mov:	call	Stick.GetStick 
		ld	(CursorData.A),bc
		or	a
		jr	z,.end1
		
		dec	a
		add	a,a
		add	a,a
		ld	hl,.move_data
		ADD_HL_A

.move:		ld	e,(hl)
		inc	hl
		ld	d,(hl)
		inc	hl
		
		push	hl
		ld	hl,(CursorData.x)
		add	hl,de
		
		ld	a,h
		cp	255
		jr	nz,.NoSubFlowX   
		ld	hl,0
		jr	.StoreX

.NoSubFlowX:	push	hl
		ld	de,(CursorData.Width)
		or	a
		sbc	hl,de
		pop	hl
				
		jr	c,.StoreX		
		ld	hl,(CursorData.Width)
		
.StoreX:	ld	(CursorData.x),hl
		pop	hl	
				
		ld	e,(hl)
		inc	hl
		ld	d,(hl)
		inc	hl
		
		ld	hl,(CursorData.y)
		add	hl,de    
		
		ld	a,h
		cp	a,255
		jr	nz,.NoSubFlowY
		ld	hl,0
		jr	.StoreY
			
.NoSubFlowY:	push	hl
		or	a
		ld	de,(CursorData.Height)
		sbc	hl,de
		pop	hl
		jr	c,.StoreY
		ld	hl,(CursorData.Height)
		
.StoreY:	ld	(CursorData.y),hl

.end1:       
		ld	hl,(CursorData.y)	
		ld	de,(CursorData.yOffset)
		add	hl,de
		ex	de,hl	
		
		push	de	
		ld	hl,(CursorData.x)
		ld	de,(CursorData.xOffset)
		add	hl,de
		pop	de
		CALL	G9k.SetCursorXY
		
.OldHook        ds	5
	
				
.move_data:	dw	0,-4	;up
		dw	4,-4	;up+right
		dw	4,0	;right
		dw	4,4	;right+down
		dw	0,4	;down
		dw	-4,4	;down+left
		dw	-4,0	;left
		dw	-4,-4	;left+up

.move_data_mouse:	dw	0,0



CursorData:	Cursor		
		ENDIF
		
