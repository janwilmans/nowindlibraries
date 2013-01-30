#ifndef _G9KLIB_H
#define _G9KLIB_H

typedef unsigned short WORD;
typedef unsigned char BYTE; 

//---------------------------------------------------------------------------//
// V9990 register and port defines                                           //
//---------------------------------------------------------------------------//

// Port defines
#define G9K_VRAM		0x60	// R/W
#define G9K_PALETTE		0x61	// R/W
#define G9K_CMD_DATA		0x62	// R/W
#define G9K_REG_DATA		0x63	// R/W
#define G9K_REG_SELECT		0x64	// W
#define G9K_STATUS		0x65	// R
#define G9K_INT_FLAG		0x66	// R/W
#define G9K_SYS_CTRL		0x67	// W
#define G9K_OUTPUT_CTRL         0x6F    // R/W

// Bit defines #define G9K_SYS_CTRL
#define G9K_SYS_CTRL_SRS	2	// Power on reset state
#define G9K_SYS_CTRL_MCKIN	1	// Select MCKIN terminal
#define G9K_SYS_CTRL_XTAL	0	// Select XTAL

// Register defines
#define G9K_WRITE_ADDR		0	// W
#define G9K_READ_ADDR		3	// W
#define G9K_SCREEN_MODE0	6	// R/W
#define G9K_SCREEN_MODE1	7	// R/W
#define G9K_CTRL		8	// R/W
#define G9K_INT_ENABLE          9       // R/W
#define G9K_INT_V_LINE_LO	10	// R/W	
#define G9K_INT_V_LINE_HI	11	// R/W
#define G9K_INT_H_LINE		12	// R/W	
#define G9K_PALETTE_CTRL	13	// W
#define G9K_PALETTE_PTR		14	// W
#define G9K_BACK_DROP_COLOR 	15      // R/W
#define G9K_DISPLAY_ADJUST	16	// R/W
#define G9K_SCROLL_LOW_Y	17      // R/W
#define G9K_SCROLL_HIGH_Y	18      // R/W
#define G9K_SCROLL_LOW_X	19      // R/W
#define G9K_SCROLL_HIGH_X	20      // R/W
#define G9K_SCROLL_LOW_Y_B	21      // R/W
#define G9K_SCROLL_HIGH_Y_B	22      // R/W
#define G9K_SCROLL_LOW_X_B	23      // R/W
#define G9K_SCROLL_HIGH_X_B	24      // R/W
#define G9K_PAT_GEN_TABLE   	25      // R/W
#define G9K_LCD_CTRL        	26      // R/W
#define G9K_PRIORITY_CTRL  	27      // R/W
#define G9K_SPR_PAL_CTRL	28	// W
#define G9K_SC_X		32	// W
#define G9K_SC_Y		34	// W
#define G9K_DS_X		36	// W
#define G9K_DS_Y		38	// W
#define G9K_NX			40	// W
#define G9K_NY			42	// W
#define G9K_ARG			44	// W
#define G9K_LOP			45	// W
#define G9K_WRITE_MASK		46	// W
#define G9K_FC			48	// W
#define G9K_BC			50	// W
#define G9K_OPCODE		52	// W

// Register Select options
#define G9K_DIS_INC_READ	64
#define G9K_DIS_INC_WRITE	128

// Bit defines G9K_SCREEN_MODE0 (register 6)
#define G9K_SCR0_STANDBY	192	// Stand by mode
#define G9K_SCR0_BITMAP		128	// Select Bit map mode
#define G9K_SCR0_P2		64	// Select P1 mode
#define G9K_SCR0_P1		0	// Select P1 mode
#define G9K_SCR0_DTCLK		32	// Master Dot clock not divided
#define G9K_SCR0_DTCLK2		16	// Master Dot clock divided by 2
#define G9K_SCR0_DTCLK4		0	// Master Dot clock divided by 4
#define G9K_SCR0_XIM2048	12	// Image size = 2048
#define G9K_SCR0_XIM1024	8	// Image size = 1024
#define G9K_SCR0_XIM512		4	// Image size = 512
#define G9K_SCR0_XIM256		0	// Image size = 256
#define G9K_SCR0_16BIT		3	// 16 bits/dot
#define G9K_SCR0_8BIT		2	// 8 bits/dot
#define G9K_SCR0_4BIT		1	// 4 bits/dot
#define G9K_SCR0_2BIT		0	// 2 bits/dot

// Bit defines G9K_SCREEN_MODE1 (register 7)
#define G9K_SCR1_C25M		64	// Select 640*480 mode
#define G9K_SCR1_SM1		32	// Selection of 263 lines during non interlace , else 262
#define G9K_SCR1_SM		16	// Selection of horizontal frequency 1H=fsc/227.5
#define G9K_SCR1_PAL		8	// Select PAL, else NTSC
#define G9K_SCR1_EO		4	// Select of vertical resoltion of twice the non-interlace resolution
#define G9K_SCR1_IL		2	// Select Interlace
#define G9K_SCR1_HSCN		1	// Select High scan mode

// Bit defines G9K_CTRL  (Register 8)
#define G9K_CTRL_DISP		128	// Display VRAM
#define G9K_CTRL_DIS_SPD	64	// Disable display sprite (cursor)
#define G9K_CTRL_YSE		32	// /YS Enable
#define G9K_CTRL_VWTE		16	// VRAM Serial data bus control during digitization
#define G9K_CTRL_VWM		8	// VRAM write control during digitization
#define G9K_CTRL_DMAE		4	// Enable DMAQ output
#define G9K_CTRL_VRAM512	2	// VRAM=512KB
#define G9K_CTRL_VRAM256	1	// VRAM=256KB
#define G9K_CTRL_VRAM128	0	// VRAM=128KB

// Bit defines G9K_INT_ENABLE (register 9)
#define G9K_INT_IECE	        4       // Command end interrupt enable control
#define G9K_INT_IEH	        2       // Display position interrupt enable
#define G9K_INT_IEV	        1       // Int. enable during vertical retrace line interval

// Bit Defines G9K_PALETTE_CTRL  (Register 13)
#define G9K_PAL_CTRL_YUV	192	// YUV mode
#define G9K_PAL_CTRL_YJK	128	// YJK mode
#define G9K_PAL_CTRL_256	64	// 256 color mode
#define G9K_PAL_CTRL_PAL	0	// Pallete mode
#define G9K_PAL_CTRL_YAE	32	// Enable YUV/YJK RGB mixing mode

// Bit defines G9K_LOP           (Register 45)
#define G9K_LOP_TP		16
#define G9K_LOP_WCSC		12
#define G9K_LOP_WCNOTSC		3
#define G9K_LOP_WCANDSC		8
#define G9K_LOP_WCORSC		14
#define G9K_LOP_WCEORSC		6

// Bit defines G9K_ARG
#define G9K_ARG_MAJ		1
#define G9K_ARG_NEG		2
#define G9K_ARG_DIX		4
#define G9K_ARG_DIY		8

// Blitter Commands G9K_OPCODE    (Register 52)
#define G9K_OPCODE_STOP		0x00
#define G9K_OPCODE_LMMC		0x10
#define G9K_OPCODE_LMMV		0x20
#define G9K_OPCODE_LMCM		0x30
#define G9K_OPCODE_LMMM		0x40
#define G9K_OPCODE_CMMC		0x050
#define G9K_OPCODE_CMMK		0x060
#define G9K_OPCODE_CMMM		0x070
#define G9K_OPCODE_BMXL		0x080
#define G9K_OPCODE_BMLX		0x090
#define G9K_OPCODE_BMLL		0x0A0
#define G9K_OPCODE_LINE		0x0B0
#define G9K_OPCODE_SRCH		0x0C0
#define G9K_OPCODE_POINT	0x0D0
#define G9K_OPCODE_PSET		0x0E0
#define G9K_OPCODE_ADVN		0x0F0

// Bit defines G9K_STATUS
#define G9K_STATUS_TR           128
#define G9K_STATUS_VR           64
#define G9K_STATUS_HR           32
#define G9K_STATUS_BD           16
#define G9K_STATUS_MSC          4
#define G9K_STATUS_EO           2
#define G9K_STATUS_CE           1

// Mode select enum for SetScreenMode
typedef enum
{
 G9K_MODE_P1,	// Pattern mode 0 256 212
 G9K_MODE_P2,	// Pattern mode 1 512 212
 G9K_MODE_B1,	// Bitmap mode 1 256 212
 G9K_MODE_B2,	// Bitmap mode 2 384 240
 G9K_MODE_B3,	// Bitmap mode 3 512 212
 G9K_MODE_B4,	// Bitmap mode 4 768 240
 G9K_MODE_B5,	// Bitmap mode 5 640 400 (VGA)
 G9K_MODE_B6,	// Bitmap mode 6 640 480 (VGA)
 G9K_MODE_B7,	// Bitmap mode 7 1024 212 (Undocumented v9990 mode)
}G9K_MODE;

// Fixed VRAM addresses
#define G9K_SCRA_PAT_NAME_TABLE 0x07C000
#define G9K_SCRB_PAT_NAME_TABLE 0x07E000
#define G9K_P1_SPR_ATTRIB_TABLE 0x03FE00
#define G9K_P2_SPR_ATTRIB_TABLE 0x07BE00

#define G9K_CURSOR0_ATTRIB      0x07FE00
#define G9K_CURSOR1_ATTRIB      0x07FE08

#define G9K_CURSOR0_PAT_DATA    0x07FF00
#define G9K_CURSOR1_PAT_DATA    0x07FF80

#define G9K_RED                 32
#define G9K_GREEN               1024
#define G9K_BLUE                1

//---------------------------------------------------------------------------//
// G9K Structs                                                               //
//---------------------------------------------------------------------------//

typedef struct _G9B_OBJECT
{
	WORD	fileHandle;	// Dos2 file handle from the openend G9B file
	BYTE	bitDepth;	// 2,4,8 or 16 bit
	BYTE	colorType;	// 0=64 color palette mode,64=256 color fixed ,128=YJK and 192=YUV mode
	BYTE	nrColors; 	// Number of colors in palette mode
	WORD	width;		// Width
	WORD	height;	        // Height
	BYTE	compression;	// 0 = no compression, other value = compression used
	BYTE	dataSize[3];    // 24 bit data size	
}G9B_OBJECT;

typedef struct _VFF_OBJECT
{
	WORD	fileHandle;	// Dos2 file handle from the openend G9B file		
	char	name[16];	// vff font name	
	BYTE    width;          // width 
	BYTE	height;		// height
	BYTE	pitch;		// 0 = Fixed : 1 = Variable  (Still unsused)
	WORD	dataSize;	// Data size
	WORD	ptrOffsetTable; // Pointer to a font offset table
	WORD	vramOffset;	// Pointer to base address of font in vram starting at #70000 
}VFF_OBJECT;

typedef struct _G9K_BOX
{
	WORD 	left;            
	WORD 	top;	            
	WORD 	width;           
	WORD 	height;
}G9K_BOX;	          
             
typedef struct _G9K_COPY_XY_XY 
{
	WORD    sourceX;         
	WORD    sourceY;             
	WORD    destX;           
	WORD    destY;           
	WORD    width;           
	WORD    height;          
}G9K_COPY_XY_XY; 
               
typedef struct _G9K_COPY_VRAM_XY
{
	BYTE	sourceAddress[3];   
	WORD	destX;		
	WORD	destY;		
	WORD	width;		
	WORD	height;				
}G9K_COPY_VRAM_XY;

typedef struct	_G9K_COPY_XY_VRAM
{
	WORD	sourceX;         
	WORD	sourceY;             		
	BYTE 	destAddress[3];     
	WORD	width;		
	WORD	height;
}G9K_COPY_XY_VRAM;					
				
//----------------------------------------------------------------------------//
// G9K Error codes                                                            //
//----------------------------------------------------------------------------//
			
// Error codes
#define _NOVFF   1  //  Input file is not a VFF file
#define _NOG9B	 2  //  Input file is not a G9B file

//----------------------------------------------------------------------------//
// General Functions overview                                                 //
//----------------------------------------------------------------------------//

// Function    : G9kReset
// Description : Reset and initialize the Gfx9000
// Input       :
// Output      :
// return value:
// Note        :
void G9kReset(void);

// Function    : G9kSetScreenMode
// Description : Set screen mode
// Input       :
// Output      :
// return value:
// Note:   	    
void G9kSetScreenMode(G9K_MODE mode);

// Function    : G9kSetVramWrite
// Description : Set vram write address
// Input       :
// Output      :
// return value:
// Note        :   	 
//void G9kSetVramWrite(); 

// Function    : G9kSetVramWrite
// Description : Set vram read address
// Input       :
// Output      :
// return value:
// Note        :  		
//void G9kSetVramRead();

// Function    : G9kDetect
// Description : Detect presence of the Gfx9000
// Input       :
// Output      :
// return value:
// Note        :  
BYTE G9kDetect(void); 

// Function    : G9kDisplayEnable
// Description : Enable display
// Input       :
// Output      :
// return value:
// Note        :  
void G9kDisplayEnable(void); 

// Function    : G9kDisplayDisable
// Description : Disable display
// Input       :
// Output      :
// return value:
// Note        :  
void G9kDisplayDisable(void); 

// Function    : G9kSpritesEnable
// Description : Enable sprites/mouse cursor
// Input       :
// Output      :
// return value:
// Note        :  
void G9kSpritesEnable(void);

// Function    : G9kSpritesEnable
// Description : Disable sprites/mouse cursor
// Input       :
// Output      :
// return value:
// Note        :  
void G9kSpritesDisable(void);

// Function    : G9kWritePalette
// Description : Write palette data to the Gfx9000
// Input       : palPtrOffset
//             : nrBytes
//             : pPalData 
// Output      :
// return value:
// Note        : 
void G9kWritePalette(BYTE palPtrOffset,BYTE nrBytes,BYTE* pPalData);

// Function    : G9kWritePalette
// Description : Read palette data to the Gfx9000
// Input       : palPtrOffset
//             : nrBytes
//             : pPalData 
// Output      :
// return value:
// Note        : 
void G9kReadPalette(BYTE palPtrOffset,BYTE nrBytes,BYTE* pPalData); 
 
// Function    : G9kSetAdjust
// Description : Adjust Gfx9000 display 
// Input       : horzAdjust
//             : vertAdjust
// Output      :
// return value:
// Note        : 
void G9kSetAdjust(char horzAdjust,char vertAdjust);

// Function    : G9kSetAdjust
// Description : Set backdrop color
// Input       : backDropColor
// Output      :
// return value:
// Note        : 
void G9kSetBackDropColor(BYTE backDropColor); 
	
// Function    : G9kSetScrollX
// Description : Set scroll X Layer A
// Input       : value
// Output      :
// return value:
// Note        : 	
void G9kSetScrollX(WORD value);
 
// Function    : G9kSetScrollY
// Description : Set scroll Y Layer A
// Input       : value
// Output      :
// return value:
// Note        : 
void G9kSetScrollY(WORD value);

// Function    : G9kSetScrollX
// Description : Set scroll X Layer B
// Input       : value
// Output      :
// return value:
// Note        : 	
void G9kSetScrollXB(WORD value);
 
// Function    : G9kSetScrollY
// Description : Set scroll Y Layer B
// Input       : value
// Output      :
// return value:
// Note        : 
void G9kSetScrollYB(WORD value);    
		
// Function    : G9kSetScrollMode
// Description : Set scroll mode Layer A
// Input       : 
// Output      :
// return value:
// Note        : 		
void G9kSetScrollMode(BYTE scrollMode);

// Function    : G9kClose
// Description : Closes a G9B or VFF file
// Input       : pObject
// Output      :
// return value:
// Note        : 
BYTE G9kClose(void* pObject); 

//----------------------------------------------------------------------------//
// Gfx9000 bitmap functions                                                   //
//----------------------------------------------------------------------------//

// Function    : G9kOpenG9B
// Description : Open a G9B file
// Input       : 
// Output      :
// return value:
// Note    
BYTE G9kOpenG9B(char* fileName,G9B_OBJECT* pG9bObject); 

// Function    : G9kReadG9B
// Description : Read data from disk to Gfx9000 VRAM X,Y
// Input       : 
// Output      :
// return value:
// Note    
BYTE G9kReadG9B(G9B_OBJECT* pG9bObject,WORD x,WORD y,BYTE palPtrOffset,WORD bufferSize,BYTE* buffer); 

// Function    : G9kReadG9BLinear
// Description : Read data from disk to Gfx9000 Linear VRAM Address
// Input       : pObject
// Output      :
// return value:
// Note    
BYTE G9kReadG9BLinear(G9B_OBJECT* pG9bObject,ULONG address,BYTE palPtrOffset,WORD bufferSize,BYTE* buffer);  




#endif