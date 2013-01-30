		MODULE	DriveInfo
		
		include "driveInfo.inc"
;----------------------------------------------------------------------------;
; Drive info functions	                                                     ;
;----------------------------------------------------------------------------;

GetDriveTypes:
; Input IY = pointer to an array of 8 DISK_INFO structs
	
		PUSH	IY
		CALL	ScanDrives	; First scan drives the dirty way
		POP	IY
		
		PUSH	IY
		CALL	CheckRamDisk    ; Check if there is a RAMDISK Avialable
		POP	IY
		
		JP	VerifyDrives    ; Verify drives with BDOS call
		


ScanDrives:
		LD	B,8
		LD	C,0 ; current drive number
		LD	IX,DPR_PTR_DRIVE_A ; Get pointer to dpr pointer table 	
.next:		
		LDI	HL,(IX)  ; Get Pointer to DPR
		LD	D,NOT_CONNECTED
		LD	A,H
		OR	A,L	 ; Check if pointer is valid
		JR	Z,.setDevType
		
		INC	HL       ; Skip drive number
		LD	A,(HL)   ; Get media type byte
		
		LD	D,FDD_DEVICE
		CP	A,0F9h	 ; Check DS Disk
		JR	Z,.checkVirtual
		LD	D,HDD_DEVICE
		CP	A,0F0H	 ; Check if HDD/ZIP/CF etc
		JR	Z,.setDevType
		LD	D,CD_ROM_DEVICE
		CP	A,0FFH	 ; CD-ROM /Ramdisk is same but it not detected by this mechanism
		JR	Z,.setDevType
		LD	D,FDD_DEVICE ; ignore ss disk
		CP	A,0F8h    ; HDD (PC media type byte) or SS Disk
		JR	Z,.setDevType
		LD	D,UNKOWN_DEVICE
.setDevType:		 
		LD	(IY+DISK_INFO.deviceType),D  ; Store device type
		LD	DE,DISK_INFO
		ADD	IY,DE
		INC	C
		DJNZ	.next
		RET


.checkVirtual:
		PUSH	DE
		PUSH	BC
		LD	B,4
		LD	HL,DRV_CTRL_1
		LD	A,C
.nextCntrller:	
		LD	C,(HL)			; Get number of drives
		INC	HL
		CP	A,C		 
		JR	C,.foundSlot            ; if carry is not set the drive is on this controller
		INC	HL
		SUB	A,C	
		DJNZ	.nextCntrller	
		; Error
		LD	A,NOT_CONNECTED		
		JR	.exit
		
.foundSlot:	
; Input A: drive number on current controller	
		; 	B => drive number on current controller
		
		LD	B,A
		LD	A,(HL)			; Get slot addres  SSPP  = secondair, primair
		LD	L,A
		AND	A,%00001100	
		LD	C,A
		LD	A,L
		
		; (pri*16+sec*4+pagenumber)*2
		AND	A,%00000011		
		ADD	A,A
		ADD	A,A
		ADD	A,A
		ADD	A,A ; Pri * 16
		ADD	A,C ; + sec*4	( sec is already * 4 )
		INC	A	; + Page numer
		LD	L,A
		LD	H,0
		ADD	HL,HL ; * 2
		LD	DE,SLTWORK_BASE;+2
		ADD	HL,DE
		LD	DE,(HL)			; Get slotwork pointer
		LD	HL,DISK_WORK_AREA.nrPhysDrives
		ADD	HL,DE
		LD	A,B	
		LD	B,(HL)
		CP	A,B		;   currentdrive on controller - total physical drives on controller
		LD	A,FDD_DEVICE
		JR	C,.exit		
		LD	A,NOT_CONNECTED		
.exit:
		POP	BC
		POP	DE	
		LD	D,A		
		JR 	.setDevType
		
CheckRamDisk:
		LD	B,GET_RAMDISK_SIZE
		BdosCall _RAMD
		DEC	B
		INC	B  ; Check B zero
		RET	Z  ; No ramdisk defined
		
		LD	DE,DISK_INFO*7	; Drive H is ramdisk
		LD	HL,IY
		ADD	HL,DE
		LD	(HL),RAM_DISK_DEVICE     ; Set drive 8 on ram disk type
		PUSH	HL		
		LD	IX,fib
		LD	DE,.ramDiskId
		LD	B,FILE_VOLUME_NAME
		CALL	Bdos.FindFirst
		POP	HL
		INC	HL	; Volume Name
		LD	BC,13
		LD	HL,fib.fileName
		LDIR	; Copy volume name
		RET

.ramDiskId:	DB	"H:\\",0


VerifyDrives:
; In some cases the dpr indicates that a drive is available, this makes sure it doesn't detect existing drives.
		BdosCall _LOGIN
		LD	A,L
		LD	B,8
.loop:
		RRCA
		JR	C,.exists
		LD	(IY+DISK_INFO.deviceType),NOT_CONNECTED
.exists:
		LD	DE,DISK_INFO
		ADD	IY,DE
		DJNZ    .loop
		RET
		
		ENDMODULE