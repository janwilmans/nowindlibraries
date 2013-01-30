		ifndef BDOS_INC
		define BDOS_INC

; System entry points
BDOS 		EQU	5h	;   BDOS entry address

; Bdos functions (DOS2)
_TERM0 		EQU	0h	;   Program terminate
_CONIN 		EQU	1h	;   Console input
_CONOUT 	EQU	2h	;   Console output
_AUXIN 		EQU	3h	;   Auxiliary input
_AUXOUT 	EQU	4h	;   Auxiliary output
_LSTOUT 	EQU	5h	;   Printer output
_DIRIO 		EQU	6h	;   Direct console I/O
_DIRIN 		EQU	7h	;   Direct console input
_INNOE 		EQU	8h	;   Console input without echo
_STROUT 	EQU	9h	;   String output
_BUFIN 		EQU	0Ah	;   Buffered line input
_CONST 		EQU	0Bh	;   Console status
_CPMVER 	EQU	0Ch	;   Return version number
_DSKRST 	EQU	0Dh	;   Disk reset
_SELDSK 	EQU	0Eh	;   Select disk
_LOGIN          EQU     18h     ;   Get login vector
_CURDRV 	EQU	19h	;   Get current drive
_SETDTA		EQU	1Ah	;   Set disk transfer address
_ALLOC		EQU	1Bh	;   Get alloc information
_GDATE 		EQU	2Ah	;   Get date
_SDATE 		EQU	2Bh	;   Set date
_GTIME 		EQU	2Ch	;   Get time
_STIME 		EQU	2Dh	;   Set time
_VERIFY 	EQU	2Eh	;   Set/reset verify flag
_RDABS 		EQU	2Fh	;   Absolute sector read
_WRABS 		EQU	30h	;   Absolute sector write
_DPARM 		EQU	31h	;   Get disk parameters
_FFIRST 	EQU	40h	;   Find first entry
_FNEXT 		EQU	41h	;   Find next entry
_FNEW 		EQU	42h	;   Find EQU entry
_OPEN 		EQU	43h	;   Open file handle
_CREATE 	EQU	44h	;   Create file handle
_CLOSE 		EQU	45h	;   Close file handle
_ENSURE 	EQU	46h	;   Ensure file handle
_DUP 		EQU	47h	;   Duplicate file handle
_READ 		EQU	48h	;   Read from file handle
_WRITE 		EQU	49h	;   Write to file handle
_SEEK 		EQU	4Ah	;   Move file handle pointer
_IOCTL 		EQU	4Bh	;   I/O control for devices
_HTEST 		EQU	4Ch	;   Test file handle
_DELETE 	EQU	4Dh	;   Delete file or subdirectory
_RENAME 	EQU	4Eh	;   Rename file or subdirectory
_MOVE 		EQU	4Fh	;   Move file or subdirectory
_ATTR 		EQU	50h	;   Get/set file attributes
_FTIME 		EQU	51h	;   Get/set file date and time
_HDELETE 	EQU	52h	;   Delete file handle
_HRENAME 	EQU	53h	;   Rename file handle
_HMOVE 		EQU	54h	;   Move file handle
_HATTR 		EQU	55h	;   Get/set file handle attributes
_HFTIME 	EQU	56h	;   Get/set file handle date and time
_GETDTA 	EQU	57h	;   Get disk transfer address
_GETVFY 	EQU	58h	;   Get verify flag setting
_GETCD 		EQU	59h	;   Get current directory
_CHDIR 		EQU	5Ah	;   Change current directory
_PARSE 		EQU	5Bh	;   Parse pathname
_PFILE 		EQU	5Ch	;   Parse filename
_CHKCHR 	EQU	5Dh	;   Check character
_WPATH 		EQU	5Eh	;   Get whole path string
_FLUSH 		EQU	5Fh	;   Flush disk buffers
_FORK 		EQU	60h	;   Fork a child process
_JOIN 		EQU	61h	;   Rejoin parent process
_TERM 		EQU	62h	;   Terminate with error code
_DEFAB 		EQU	63h	;   Define abort exit routine
_DEFER 		EQU	64h	;   Define disk error handler routine
_ERROR 		EQU	65h	;   Get previous error code
_EXPLAIN 	EQU	66h	;   Explain error code
_FORMAT 	EQU	67h	;   Format a disk
_RAMD 		EQU	68h	;   Create or destroy RAM disk
_BUFFER 	EQU	69h	;   Allocate sector buffers
_ASSGIN 	EQU	6Ah	;   Logical drive assignment
_GENV 		EQU	6Bh	;   Get environment item
_SENV 		EQU	6Ch	;   Set environment item
_FENV 		EQU	6Dh	;   Find environment item
_DSKCHK 	EQU	6Eh	;   Get/set disk check status
_DOSVER 	EQU	6Fh	;   Get MSX DOS version number
_REDIR 		EQU	70h	;   Get/set redirection status

; Errors
ERROR
.NCOMP 		EQU	0FFh	;   Incompatible disk
.WRERR 		EQU	0FEh	;   Write error
.DISK 		EQU	0FDh	;   Disk error
.NRDY 		EQU	0FCh	;   Not ready
.VERFY 		EQU	0FBh	;   Verify error
.DATA 		EQU	0FAh	;   Data error
.RNF 		EQU	0F9h	;   Sector not found
.WPROT 		EQU	0F8h	;   Write protected disk
.UFORM 		EQU	0F7h	;   Unformatted disk
.NDOS 		EQU	0F6h	;   Not a DOS disk
.WDISK 		EQU	0F5h	;   Wrong disk
.WFILE 		EQU	0F4h	;   Wrong disk for file
.SEEK 		EQU	0F3h	;   Seek error
.IFAT 		EQU	0F2h	;   Bad file allocation table
.NOUPB 		EQU	0F1h	;   --
.IFORM 		EQU	0F0h	;   Cannot format this drive
.INTER 		EQU	0DFh	;   Internal error
.NORAM 		EQU	0DEh	;   Not enough memory
.IBDOS 		EQU	0DCh	;   Invalid MSX-DOS call
.IDRV 		EQU	0DBh	;   Invalid drive
.IFNM 		EQU	0DAh	;   Invalid filename
.IPATH          EQU     0D9h	;   Invalid pathname
.PLONG 		EQU	0D8h	;   Pathname too long
.NOFIL 		EQU	0D7h	;   File not found
.NODIR 		EQU	0D6h	;   Directory not found
.DRFUL 		EQU	0D5h	;   Root directory full
.DKFUL 		EQU	0D4h	;   Disk full
.DUPF 		EQU	0D3h	;   Duplicate filename
.DIRE 		EQU	0D2h	;   Invalid directory move
.FILRO 		EQU	0D1h	;   Read only file
.DIRNE 		EQU	0D0h	;   Directory not empty
.IATTR 		EQU	0CFh	;   Invalid attributes
.DOT 		EQU	0CEh	;   Invalid . or .. operation
.SYSX 		EQU	0CDh	;   System file exists
.DIRX 		EQU	0CCh	;   Directory exists
.FILEX 		EQU	0CBh	;   File exists
.FOPEN 		EQU	0CAh	;   File already in use
.OV64K 		EQU	0C9h	;   Cannot transfer above 64K
.FILE 		EQU	0C8h	;   File allocation error
.EOF 		EQU	0C7h	;   End of file
.ACCV 		EQU	0C6h	;   File access violation
.IPROC 		EQU	0C5h	;   Invalid process id
.NHAND 		EQU	0C4h	;   No spare file handles
.IHAND 		EQU	0C3h	;   Invalid file handle
.NOPEN 		EQU	0C2h	;   File handle not open
.IDEV 		EQU	0C1h	;   Invalid device operation
.IENV 		EQU	0C0h	;   Invalid environment string
.ELONG 		EQU	0BFh	;   Environment string too long
.IDATE 		EQU	0BEh	;   Invalid date
.ITIME 		EQU	0BDh	;   Invalid time
.RAMDX 		EQU	0BCh	;   RAM disk (drive H ) already exists
.NRAMD 		EQU	0BBh	;   RAM disk does not exist
.HDEAD 		EQU	0BAh	;   File handle has been deleted
.EOL 		EQU	0B9h	;   Internal error. Should never occur
.ISBFN 		EQU	0B8h	;   Invalid sub-function number
.STOP 		EQU	09Fh	;   Ctrl-STOP pressed
.CTRLC 		EQU	09Eh	;   Ctrl-C pressed
.ABORT 		EQU	09Dh	;   Disk operation aborted
.OUTERR 	EQU	09Ch	;   Error on standard output
.INERR 		EQU	09Bh	;   Error on standard input
.BADCOM 	EQU	08Fh	;   Wrong version of COMMAND
.BADCM 		EQU	08Eh	;   Unrecognized command
.BUFUL 		EQU	08Dh	;   Command too long
.OKCMD 		EQU	08Ch	;   --
.IPARM 		EQU	08Bh	;   Invalid parameter
.INP 		EQU	08Ah	;   Too many parameters
.NOPAR 		EQU	089h	;   Missing parameter
.IOPT 		EQU	088h	;   Invalid option
.BADNO 		EQU	087h	;   Invalid number
.NOHELP 	EQU	086h	;   File for HELP not found
.BADVER 	EQU	085h	;   Wrong version of MSX-DOS
.NOCAT 		EQU	084h	;   Cannot concatenate destination file
.BADEST 	EQU	083h	;   Cannot create destination file
.COPY 		EQU	082h	;   File cannot be copied onto itself
.OVDEST 	EQU	081h	;   Cannot overwrite previous destination file

; Allocate segment parameters
USER_SEGMENT	   	EQU     0	; Register A
SYSTEM_SEGMENT     	EQU     1    ;   "         "

PRIMARY_MAPPER     	EQU     0    ; Register B 

; Open/create file flags (_OPEN)
OPEN_NO_WRITE	   	EQU     1
OPEN_NO_READ       	EQU     2
OPEN_INHERITABLE   	EQU     4 

; Seek flags  (_SEEK)
SEEK_REL_BEGINING    	EQU 	0   ; Relative to the beginning of the file
SEEK_REL_CURRENT	EQU     1   ; Relative to the current position
SEEK_REL_END		EQU	2   ; Relative to the end of the file.

; Create flags
CREATE_NEW	   	EQU     128

; File attributes
FILE_READ_ONLY	   	EQU     1   ; 
FILE_HIDDEN	   	EQU     2   ;
FILE_SYSTEM	   	EQU     4   ; 
FILE_VOLUME_NAME   	EQU     8   ; 
FILE_DIRECTORY     	EQU     16  ; 
FILE_ARCHIVE	   	EQU     32  ; 
FILE_RESERVED      	EQU     64  ; 
FILE_DEVICE	   	EQU    128  ;

; Disk error handler routine parameters in register C
ERROR_WRITING	   	EQU	1  ; Set if writing
ERROR_NO_IGNORE    	EQU     2  ; Set if ignore not recommended
ERROR_AUTO_ABORT   	EQU     4  ; Set if auto-abort suggested
ERROR_VALID_SECTOR 	EQU     8  ; Set if sector number is valid 


; Disk error handler routine return codes
CALL_SYSTEM_ERROR 	EQU     0 ; Call system error routine
ERROR_ABORT       	EQU     1 ; Abort
ERROR_RETRY       	EQU     2 ; Retry
ERROR_IGNORE      	EQU     3 ; Ignore


; Character flags for _CHKCHR
SUPPRESS_UPPER_CASING  	 EQU   0  ; Set to suppress upper casing
FIRST_BYTE_WIDE_CHAR	 EQU   1  ; Set if first byte of 16-bit character
SECOND_BYTE_WIDE_CHAR	 EQU   2  ; Set if first byte of 16-bit character
VOLUME_NAME		 EQU   3  ; Set if second byte of 16-bit character
NOT_A_VALID_CHAR	 EQU   4  ; Not a valid file/volume name character


;  Params _RAMD
DESTROY_RAMDISK	  	EQU	0
GET_RAMDISK_SIZE  	EQU   0FFh
CREATE_RAMDISK	  	EQU   1	;  CREATE_RAMDISK+n = 1+n

; Misc
CR 			EQU	0Dh	;   Carridge Return
LF 			EQU	0Ah	;   Line Feed
LE 			EQU	24h	;   Line end
UPPER_CASE_MASK 	EQU     0DFh

CMD_LENGTH		EQU	080h
CMD_LINE		EQU	082h

COM_START_ADDR  	EQU     0100h
/*
		STRUCT	FIB
ff		BYTE          ; 0 - Always 0FFh		
fileName	BLOCK   13,0  ; Filename as an ASCIIZ string
fileAttrib	BYTE	      ; File attributes byte
lastModTime	WORD	      ; Time of last modification
lastModDate	WORD	      ; Date of last modification
startCluster	WORD	      ; Start Cluster
fileSize	BLOCK	4,0   ; Files size
logicalDrive	BYTE	      ; Logical drive
internalInfo	BLOCK	38,0  ; Internal information, must not be modified
		ENDS


		STRUCT DISK_PAR
physicalDrive   BYTE	    ; 0      - Physical drive number (1=A: etc)
sectorSize      WORD	    ; 1,2    - Sector size (always 512 currently)
sectorsCluster  BYTE	    ; 3      - Sectors per cluster (non-zero power of 2)
reservedSectors WORD	    ; 4,5    - Number of reserved sectors (usually 1)
nrOfFats	BYTE	    ; 6      - Number of copies of the FAT (usually 2)
nrRootEntries	WORD	    ; 7,8    - Number of root directory entries
logicalSectors	WORD	    ; 9,10   - Total number of logical sectors
mediaDiscriptor BYTE	    ; 11     - Media descriptor byte
nrSectorsFat	BYTE	    ; 12     - Number of sectors per FAT
firstRootSector WORD	    ; 13..14 - First root directory sector number
firstDataSector WORD	    ; 15..16 - First data sector number
maxClusterNr    WORD	    ; 17..18 - Maximum cluster number
dirtyDisk       BYTE	    ; 19     - Dirty disk flag
volumeID	BLOCK 4,0   ; 20..23 - Volume id. (-1 => no volume id.)
reserved	BLOCK 8,0   ; 24..31 - Reserved (currently always zero)
		ENDS

*/

DPR_PTR_DRIVE_A EQU	0f355h
DPR_PTR_DRIVE_B EQU	0f357h
DPR_PTR_DRIVE_C EQU	0f359h
DPR_PTR_DRIVE_D EQU	0f35bh
DPR_PTR_DRIVE_E EQU	0f35dh
DPR_PTR_DRIVE_F EQU	0f35fh
DPR_PTR_DRIVE_G EQU	0f361h
DPR_PTR_DRIVE_H EQU	0f363h
  

/*                
		STRUCT DPR
driveNumber	BYTE	; Drive nummer
mediaId     	BYTE    ; Media ID byte
bytesPerSector  WORD    ; number of bytes per sector
dirMask		BYTE    ; directory maske
dirShift	BYTE	; directory shift
clusterMask	BYTE 	; cluster mask
clusterShift	BYTE    ; cluster shift
firstFatSector  WORD	; first FAT sector
nrFats		BYTE	; Number of FATs
nrDirEntries	BYTE    ; aantal directory entries
firstDataSector WORD    ; eerste data sector
nrClusters	WORD    ; Number of clusters + 1
sectorsFat	BYTE    ; Number of sectors per FAT
firstDirSector  WORD    ; eerste directory sector
fatAddress	WORD    ; FAT-address
		ENDS
*/

DRV_CTRL_1	 EQU 	0FB21h ; number of drives connected to the 1st interface
SLOT_DRV_CNTRL_1 EQU 	0FB22h ; slotcode of the 1st interface
DRV_CTRL_2	 EQU 	0FB23h ; number of drives connected to the 2nd interface
SLOT_DRV_CNTRL_2 EQU 	0FB24h ; slotcode of the 2nd interface
DRV_CTRL_3	 EQU 	0FB25h ; number of drives connected to the 3rd interface
SLOT_DRV_CNTRL_3 EQU 	0FB26h ; slotcode of the 3rd interface
DRV_CTRL_4	 EQU 	0FB27h ; number of drives connected to the 4th interface
SLOT_DRV_CNTRL_4 EQU 	0FB28h ; slotcode of the 4th interface


SLTWORK_BASE	 EQU    0FD09h
/*
		STRUCT DISK_WORK_AREA	
diskStopCount	BYTE ; Diskstop counter
readyFlagA	BYTE ; Ready-flag drive A: (=counter after diskstop)   
readyFlagB      BYTE ; Ready-flag drive B: (=counter after diskstop)
activePhysDrive BYTE ; Active physical drive
lastTrackVirDrA BYTE ; Last track position for virtual drive A: 
lastTrackVirDrB BYTE ; Last track position for virtual drive B: 
activeLogDrive  BYTE ; Active logical drive
nrPhysDrives    BYTE ; number of connected physical drives
		ENDS


		STRUCT MAPPER_TABLE
slotAddress	BYTE	    ; Slot address of the mapper slot.
nrSegments	BYTE	    ; Total number of 16k RAM segments. 1...255 (8...255 for the primary)		
freeSegments    BYTE        ; Number of free 16k RAM segments.
sysAllocated    BYTE        ; Number of 16k RAM segments allocated to the system (at least 6 for the primay)
userAllocated   BYTE        ; Number of 16k RAM segments allocated to the user
		ENDS	
		   
; Entries for other mapper slots. If there is none, MAPPER_TABLE+8 will be zero.

		STRUCT MAPPER_FUNCTIONS
AllocateSeg	BLOCK	3,0	;   Allocate a 16k segment.
FreeSeg		BLOCK	3,0     ;   Free a 16k segment.
ReadSeg		BLOCK	3,0	;   Read byte from address A HL to A.
WriteSeg	BLOCK	3,0	;   Write byte from E to address A HL.
CallSeg		BLOCK	3,0	;   Inter-segment call. Address in IYh IX
CallsSeg	BLOCK	3,0	;   Inter-segment call. Address in line after the call instruction.
PutSeg		BLOCK	3,0	;   Put segment into page (HL).
GetSeg		BLOCK	3,0	;   Get current segment for page (HL)
PutSeg0		BLOCK	3,0	;   Put segment into page 0.
GetSeg0		BLOCK	3,0	;   Get current segment for page 0.
PutSeg1		BLOCK	3,0	;   Put segment into page 1.
GetSeg1		BLOCK	3,0	;   Get current segment for page 1.
PutSeg2		BLOCK	3,0	;   Put segment into page 2.
GetSeg2		BLOCK	3,0	;   Get current segment for page 2.
PutSeg3		BLOCK	3,0	;   Put segment into page 3.
GetSeg3		BLOCK	3,0	;   Get current segment for page 3.
		ENDS

		 STRUCT	MAPPER
nrSegments	 BYTE			
priMapperSlot  	 BYTE
freeSegments	 BYTE
mapperSupport    WORD          ; mapper support routine address
AllocateSegment  BLOCK 3,0     ;  Allocate a 16k segment.
FreeSegment	 BLOCK 3,0     ;  Free a 16k segment.	
ReadSegmentByte  BLOCK 3,0     ;  Read byte from address A HL to A.
WriteSegmentByte BLOCK 3,0     ;  Write byte from E to address A HL.
InterSegCallReg  BLOCK 3,0     ;  Inter-segment call.     Address in IYh IX 	
InterSegCall     BLOCK 3,0     ;  Inter-segment call.     Address in line after the call instruction.
PutSegmentHL     BLOCK 3,0     ;  Put segment into page (HL).
GetSegmentHL     BLOCK 3,0     ;  Get current segment for page (HL)
PutSegment0      BLOCK 3,0     ;  Put segment into page 0.
GetSegment0      BLOCK 3,0     ;  Get current segment for page 0.
PutSegment1      BLOCK 3,0     ;  Put segment into page 1.
GetSegment1      BLOCK 3,0     ;  Get current segment for page 1.
PutSegment2      BLOCK 3,0     ;  Put segment into page 2.
GetSegment2      BLOCK 3,0     ;  Get current segment for page 2.
PutSegment3      BLOCK 3,0     ;  Not supported since page-3 must never be changed. Acts like a "NOP" if called.
GetSegment3      BLOCK 3,0     ;  Get current segment for page 3.
	 	 ENDS
*/			   
	        MACRO   BdosCall function
                LD      C,function
                CALL    BDOS
                ENDM
/*
		MACRO	CallSegment segment,address	
		LD      IY,(segment-1)       
		LD      IX,address            
		CALL    mapper.InterSegCallReg      
		ENDM
*/
		endif