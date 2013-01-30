			ifndef WAVEDRIVER_INC
			define WAVEDRIVER_INC

WVIO			equ	07eh	; wave I/O base port
FMIO			equ	0c4h
PTW_SIZE		equ	20	; size of Wave playtable line
WAVCHNS			equ	24
WAVESEG			equ	0FB1Fh	; Mapper segement for the replayer is stored at this address.

													
			STRUCT	WAVE_DRV_JUMP,04000h
LoadSong		BLOCK	3
; Load mwm file
; Input DE = Pointer to file name
;        A = bank number  			
; Output A = Error code

LoadWaveKit		BLOCK	3
; Load mwk file
; Input DE = Pointer to file name  			
; Output A = Error code

Play			BLOCK	3
; Start playing song
; Input  A = Bank numer

Stop			BLOCK	3
; Stop playing song

Continue		BLOCK	3
; Continue playing song

Fade			BLOCK	3
; Fade music
; Input A = Fade speed

AllocateSongSegments	BLOCK	3
;  Allocate segments
;  B = number of segments to allocate

FreeSongSegments	BLOCK	3
;

GetInfo			BLOCK	3
; 

SetSongAddress		BLOCK	3
; 

GetDriverVersion	BLOCK	3

; File function. Overwrite these the use external file loading 
FileOpen		BLOCK	3

FileRead		BLOCK	3

; Note: save content of register DE
FileClose		BLOCK	3

banks			BLOCK	256
DOSJmpTab		WORD
playBusy		BYTE		; status:   0 = not playing ; #ff =playing
songdataBank1		BYTE		; mapperbank with song data
songdataBank2		BYTE		; mapperbank with song data
songdataBank3		BYTE		; mapperbank with song data
songdataAddress		WORD		; address of song data (+18 bytes for XLFO data)
playPos			BYTE		; current position
playStep		BYTE		; current step
status			WORD		; status bytes (0 = off)
stepBuffer		BLOCK	25	; decrunched step, played next int
loadBuffer		BLOCK	3
playInt			BLOCK	3
			ENDS	
			
			
			STRUCT	SMART_TABLE
lastNote		BYTE  		;    - last note played
freqRegister		BYTE		;    - frequency register
pitchBend		BYTE		;    - pitch bending on/off
pitchBendSpeed		WORD		;    - pitch bend speed
detuneValue		BYTE		;    - detune value    
toneNrNextInt		WORD		;    - tone nr for next interrupt 
freqNextInt		WORD		;    - freq for next interrupt
currentPatch		BYTE		;    - current patch	
currentStereoSet	BYTE		;    - current stereo setting
pseudoReverb		BYTE		;    - pseodo reverb
ptrHeaderBytes		WORD		;    - Pointer to header bytes 
volume			BYTE		;    - Volume
ptrFreqTable		WORD		;    - Pointer to used freq table 
currentPitchFreq	WORD		;    - Current pitch freq.   
			ENDS			
		
			endif