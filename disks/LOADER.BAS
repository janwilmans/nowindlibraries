10 CLS
20 SW=PEEK(&HF3B0): 'LINLIN (screen width)
30 XM=CINT((SW-13)/13)*13
40 PRINT "[Select file to start]:":LOCATE 0,4
50 ON ERROR GOTO 120
60 'list files
70 OFS=CSRLIN
80 FILES "*.bas"
90 FILES "*.ldr
100 FILES "*.prg"
110 GOTO 130
120 RESUME NEXT:' ignore errors
130 ' done listing files
140 ON ERROR GOTO:' remove handler
150 X=0:Y=OFS:YM=CSRLIN:GOSUB 250
160 'main loop
170 GOSUB 250
180 A$=INPUT$(1):A=ASC(A$)
190 IF A=28 THEN IF X<XMTHEN X=X+13
200 IF A=29 THEN IF X>0 THEN X=X-13
210 IF A=30 THEN IF Y>OFS THEN Y=Y-1
220 IF A=31 THEN IF Y<YM THEN Y=Y+1
230 IF A=13 GOTO 290
240 GOTO 160
250 ' update screen
260 'LOCATE 30,0:PRINTUSING "keycode: ###";A
270 LOCATE X,Y,1:'set position, enable cursor
280 RETURN
290 ' run program at cursor
300 'get the filename from the screen
310 'width 37 is actually width 40 with 3 unreachable chars
320 'some calculating is needed to get the right positions for all widths
330 LOCATE 0,0,0:'remove cursor
340 IF (SW>40) THEN YW=80 ELSE YW=40
350 SM=PEEK(&HFCAF):'screen mode
360 IF (SM=0) THEN SB=PEEK(&HF3B3)+256*PEEK(&HF3B4):GOTO 380
370 IF (SM=1) THEN SB=PEEK(&HF3BD)+256*PEEK(&HF3BE):YW=32
380 ' sb = screen base address
390 YO=CINT((YW+1-SW)/2):'y-offset for width < 40 or < 80
400 B$="":FOR A=0 TO 12:B$=B$+CHR$(VPEEK((Y*YW)+X+A+SB+YO)):NEXT
410 LOCATE 0,2,0:PRINTUSING "[&]";B$
420 ON ERROR GOTO 470
430 RUN B$
440 'return from error
450 ON ERROR GOTO:'remove handler
460 GOTO 160
470 'load error
480 LOCATE 15,2:PRINT">> Load error!!":BEEP
490 RESUME 440
