#include <time.h>
#include <rtthread.h>
#include <string.h>
#include "fh_vpu_mpi.h"
#include "fh_venc_mpi.h"
#include "bufCtrl.h"
#include "fh_common.h"
#include "time_utils.h"
#include "osd.h"

//#define OSD_DEBUG
#ifdef OSD_DEBUG 
#define OSD_DBG(...) printf(__VA_ARGS__);
#else
#define OSD_DBG(...)
#endif
///////////////////////////////////////

#define MAX_OSD_REGION	(10)	

#define MAX_OSD_IMG_W				704				
#define MAX_IMG_H				576

#define MAX_CHAR_OSD_LINE		(MAX_OSD_IMG_W/8)	
#define OSD_MAX_LINE_BASE	    10	
#define MAX_ENC_CHAN_PER        4

#define OSD_TAG_CODE_BASE		0x9000					/*?????????*/
#define OSD_YEAR4				(OSD_TAG_CODE_BASE+0)	/*4λ??2002*/
#define OSD_YEAR2				(OSD_TAG_CODE_BASE+1)	/*2λ??02	*/
#define OSD_MONTH3				(OSD_TAG_CODE_BASE+2)	/*3λ?·??JAN??DEC*/
#define OSD_MONTH2				(OSD_TAG_CODE_BASE+3)	/*2λ?·??1??12*/
#define OSD_DAY					(OSD_TAG_CODE_BASE+4)	/*2λ?????1??31*/
#define OSD_WEEK3				(OSD_TAG_CODE_BASE+5)	/*3λ?????MON??SUN*/
#define OSD_CWEEK1				(OSD_TAG_CODE_BASE+6)	/*2λ??1λ???????????????*/
#define OSD_HOUR24				(OSD_TAG_CODE_BASE+7)	/*2λ24???С???00??23	*/
#define OSD_HOUR12				(OSD_TAG_CODE_BASE+8)	/*2λ12???С???00??12*/
#define OSD_MINUTE				(OSD_TAG_CODE_BASE+9)	/*2λ????00??59*/
#define OSD_SECOND				(OSD_TAG_CODE_BASE+10)	/*2λ??00??59*/
#define OSD_AMPM				(OSD_TAG_CODE_BASE+25)	/*AMPM*/

#define OSD_WEEK3_DUTCH 		(OSD_TAG_CODE_BASE+11) 	/*2λ?????MA??ZO*/
/* ???? */
#define OSD_WEEK3_FRENCH 		(OSD_TAG_CODE_BASE+12) 	/*3λ?????LUN??DIM*/
/* ?1? */
#define OSD_WEEK3_GERMAN 		(OSD_TAG_CODE_BASE+13) 	/*2λ?????MO??SO*/
/* ????? */
#define OSD_WEEK3_ITALY 		(OSD_TAG_CODE_BASE+14) 	/*3λ?????LUN??DOM*/
/* ???? */
#define OSD_WEEK3_POLISH 		(OSD_TAG_CODE_BASE+15) 	/*3λ?????PON??NIE*/
/* ?????? */
#define OSD_WEEK3_SPANISH 		(OSD_TAG_CODE_BASE+16) 	/*3λ?????LUN??DOM*/
/* ?????? */
#define OSD_WEEK3_PORTUGAL		(OSD_TAG_CODE_BASE+17) 	/*3λ????: SEG??DOM*/
/* ???? */
#define OSD_WEEK3_RUSSIA		(OSD_TAG_CODE_BASE+18) 	/*2λ????*/
/* ????? */
#define OSD_WEEK3_GREECE		(OSD_TAG_CODE_BASE+19) 	/*3λ????*/
/* ??? */
#define OSD_CWEEK1_JAPAN		(OSD_TAG_CODE_BASE+20) 	/*2λ??1λ???????????????*/
/* ?????? */
#define OSD_WEEK3_HUNGARY     	(OSD_TAG_CODE_BASE+21) 	/*3λ?????H\x82t??Vas*/
/* ??? */
#define OSD_WEEK3_CZECHISH     	(OSD_TAG_CODE_BASE+22) 	/*2λ?????St??Ne*/
/* ??工??*/
#define OSD_WEEK3_SLOVAKIAN    	(OSD_TAG_CODE_BASE+23) 	/*2λ?????St??Ne*/
/* ?????? */
#define OSD_WEEK3_TURKEY     	(OSD_TAG_CODE_BASE+24) 	/*3λ?????Paz??Cts*/

#define FONT_LIB_RS_LEN		1056
#define FONT_LIB_GREECE_LEN	768


///////////////

typedef struct
{
    short year;
    short month;
    short dayOfWeek;        /*0:??????-6:??????*/
    short day;
    short hour;
    short minute;
    short second;
    short milliSecond;
}DATE_TIME;

typedef struct
{
    FH_UINT32    charCnt;    
    FH_UINT32    top;        
    unsigned int    x;        
    unsigned int    y;        
     FH_UINT32    hScale;        
    FH_UINT32    vScale;        

    FH_UINT32    code[MAX_CHAR_OSD_LINE];
}OSD_LINE_PARAM;

typedef struct
{
    FH_UINT32    code;   
    FH_UINT32    pos;
    FH_UINT32    pos_orig;
    FH_UINT32    top;
    FH_UINT32    color;
    FH_UINT32    size;
    unsigned char *pFont;    
    FH_UINT32    tagIdx;
    FH_UINT32    charType;
    FH_UINT32    colorIdx;
    FH_UINT32    bChanged;
}OSD_CHAR;

typedef struct
{
    FH_UINT32 charCnt;
    FH_UINT32 validCharCnt;
    FH_UINT32 occupyCharCnt;
    FH_UINT32 x,y,w,h;
    FH_UINT32 osdScaleH;
    FH_UINT32 osdScaleV;
    void (*pDrawFunc)(unsigned char *pFont,unsigned char *pDst,FH_UINT32 w,FH_UINT32 h,FH_UINT32 pitch,FH_UINT32 color);
    OSD_CHAR charData[MAX_CHAR_OSD_LINE*2];
    FH_UINT32 LINE_COLOR;
}OSD_LINE;

typedef struct
{
    FH_BOOL bReady;        
    FH_BOOL bFlash;       
    FH_BOOL bAutoLum;      
    FH_BOOL bTranslucent; 
    FH_UINT32 flashOn;
    FH_UINT32 flashOff;
    OSD_LINE line[OSD_MAX_LINE_BASE];
} OSD_PARAM;

typedef struct 
{ 
    FH_UINT32            flash;                
    FH_BOOL            bAutoAdjustLum;        
    FH_UINT32            lum;                
    FH_BOOL            bTranslucent;    
    OSD_LINE_PARAM     line[OSD_MAX_LINE_BASE];    
    unsigned char     osdSize[MAX_ENC_CHAN_PER];
} OSD_CONFIG;

typedef struct
{
    FH_UINT32 charCnt;
    FH_UINT32 charWidth;
    FH_UINT32 flgEncUsed;
    OSD_CHAR *pFirstChar[MAX_ENC_CHAN_PER];
    FH_UINT32 line;
    FH_UINT32 bRefresh;
} OSD_TAG_PARAM;

#define OSD_WHITE_COLOR            (0xff88)
#define OSD_TAG_CNT                80
#define    OSD_SIZE_16_16            0
#define    OSD_SIZE_32_32            1
#define    OSD_SIZE_64_64            2
#define OSD_SIZE_AUTO            0XFF         
#define OSD_CHAR_W_ASC            8
#define OSD_CHAR_W_HZ            16

#define FONT_ASC_LEN        4096
#define FONT_HZ_SIMPL_LEN    267616
#define FONT_HZ_GBK_LEN        766080
#define LOGO_MAX_W        128 /*???*/
#define LOGO_MAX_H        128 /*???*/


//////////////////

#define TIME_OSD0_NORCOLOR 0xffff8080 /* ????? 0xff88 white */
#define TIME_OSD0_INVCOLOR 0xff008080 /* ?????? 0xf088 black */

#define TIMER_PRICISION_DIST (10)        /*?????   10ms????ж?*/

#define OSD_CHAR_TYPE_ASC        0
#define OSD_CHAR_TYPE_HZ        1
#define OSD_CHAR_TYPE_RS        2        /* ???? */
#define OSD_CHAR_TYPE_TAG        3
#define OSD_CHAR_TYPE_GREECE    4        /* ????? */
#define OSD_CHAR_TYPE_SPECIAL  5        /*???????*/
#define OSD_CHAR_CNT    (32*OSD_MAX_LINE_BASE)

#define GET_ASC_OFFSET(x) ((x)*16)
#define GET_ONE(x) ((x)%10)
#define GET_TEN(x) (((x)%100)/10)
#define GET_HUNDRED(x) (((x)%1000)/100)
#define GET_THOUSAND(x) (((x)%10000)/1000)

#define TEST_YEAR(x) ((x & 3) ? 0 : (x % 100 ? 1 : (x % 400 ? 0 : 1))) /*  ?????ж? */


#define FONT_LIB_RS_LEN        1056
#define FONT_LIB_GREECE_LEN    768    
#define OSD_INVERT_VALUE   140


static int osdTimeFlush = 0; // rzn 2015.10.28 add

static int rotate_flag ;
static int rotate_value ;
static const unsigned char INDEX_C[32] = {\
    8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,\
};

static const unsigned char INDEX_C_ROTATE[32] = {
    39,38,37,36,35,34,33,32,31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,\
};

static unsigned char INDEX_C2[256] = {
    8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,\
    40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,\
    72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,\
    104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,128,129,130,131,132,133,134,135,\
    136,137,138,139,140,141,142,143,144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,160,161,162,163,164,165,166,167,\
    168,169,170,171,172,173,174,175,176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,192,193,194,195,196,197,198,199,\
    200,201,202,203,204,205,206,207,208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,224,225,226,227,228,229,230,231,\
    232,233,234,235,236,237,238,239,240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255,255,255,255,255,255,255,255,255,\
};

static unsigned char INDEX_C2_90[256] = 
{
    39,    71,    103,135,167,199,231,255,\
    38,    70,    102,134,166,198,230,255,\
    37,    69,    101,133,165,197,229,255,\
    36,    68,    100,132,164,196,228,255,\
    35,    67,    99,    131,163,195,227,255,\
    34,    66,    98,    130,162,194,226,255,\
    33,    65,    97,    129,161,193,225,255,\
    32,    64,    96,    128,160,192,224,255,\
    31,    63,    95,    127,159,191,223,255,\
    30,    62,    94,    126,158,190,222,254,\
    29,    61,    93,    125,157,189,221,253,\
    28,    60,    92,    124,156,188,220,252,\
    27,    59,    91,    123,155,187,219,251,\
    26,    58,    90,    122,154,186,218,250,\
    25,    57,    89,    121,153,185,217,249,\
    24,    56,    88,    120,152,184,216,248,\
    23,    55,    87,    119,151,183,215,247,\
    22,    54,    86,    118,150,182,214,246,\
    21,    53,    85,    117,149,181,213,245,\
    20,    52,    84,    116,148,180,212,244,\
    19,    51,    83,    115,147,179,211,243,\
    18,    50,    82,    114,146,178,210,242,\
    17,    49,    81,    113,145,177,209,241,\
    16,    48,    80,    112,144,176,208,240,\
    15,    47,    79,    111,143,175,207,239,\
    14,    46,    78,    110,142,174,206,238,\
    13,    45,    77,    109,141,173,205,237,\
    12,    44,    76,    108,140,172,204,236,\
    11,    43,    75,    107,139,171,203,235,\
    10,    42,    74,    106,138,170,202,234,\
    9,    41,    73,    105,137,169,201,233,\
    8,    40,    72,    104,136,168,200,232,\
};

static unsigned char INDEX_C2_180[256];
static unsigned char INDEX_C2_270[256];
#define MAX_TEXT_LINE 3
#define OSD_TEXT0  (1 << 5)
#define OSD_TEXT1  (1 << 6)
#define OSD_TEXT2  (1 << 7)

/*
    OSD??????
    code????????????????????????????????????????????OSD?????????????
    pos??λ???OSD?????X??Y??W??H?????????????????????
        bit  0- 15:X(?????)
        bit  16-31:Y(?????)
    top??λ???OSD?????
        bit 0-15:X(????????????????????仯)
        bit 16-31:Y(????????????????????仯)
    size:??С??
        bit  0- 7:W(?????)
        bit  8-15:H(?????)
        bit 16-23:W(????????????????????仯)
        bit 24-31:H(????????????????????仯)
*/
static OSD_TAG_PARAM osdTagParam[OSD_TAG_CNT]=
{
    {4,OSD_CHAR_W_ASC,0,{0},0,0},\
    {2,OSD_CHAR_W_ASC,0,{0},0,0},\
    {3,OSD_CHAR_W_ASC,0,{0},0,0},\
    {2,OSD_CHAR_W_ASC,0,{0},0,0},\
    {2,OSD_CHAR_W_ASC,0,{0},0,0},\
    {3,OSD_CHAR_W_ASC,0,{0},0,0},\
    {3,OSD_CHAR_W_HZ ,0,{0},0,0},\
    {2,OSD_CHAR_W_ASC,0,{0},0,0},\
    {2,OSD_CHAR_W_ASC,0,{0},0,0},\
    {2,OSD_CHAR_W_ASC,0,{0},0,0},\
    {2,OSD_CHAR_W_ASC,0,{0},0,0},\
    {2,OSD_CHAR_W_ASC,0,{0},0,0},\
    {3,OSD_CHAR_W_ASC,0,{0},0,0},\
    {2,OSD_CHAR_W_ASC,0,{0},0,0},\
    {3,OSD_CHAR_W_ASC,0,{0},0,0},\
    {3,OSD_CHAR_W_ASC,0,{0},0,0},\
    {3,OSD_CHAR_W_ASC,0,{0},0,0},\
    {3,OSD_CHAR_W_ASC,0,{0},0,0},\
    {2,OSD_CHAR_W_ASC,0,{0},0,0},\
    {3,OSD_CHAR_W_ASC,0,{0},0,0},\
    {1,OSD_CHAR_W_HZ ,0,{0},0,0},\
    {3,OSD_CHAR_W_ASC,0,{0},0,0},\
    {2,OSD_CHAR_W_ASC,0,{0},0,0},\
    {2,OSD_CHAR_W_ASC,0,{0},0,0},\
    {3,OSD_CHAR_W_ASC,0,{0},0,0},\
    {2,OSD_CHAR_W_ASC,0,{0},0,0}\
};


#define CHANNEL_NO_TEST  0

static const  unsigned char weeksNameEng[7][3]=
{
    {'S','u','n'},\
    {'M','o','n'},\
    {'T','u','e'},\
    {'W','e','d'},\
    {'T','h','u'},\
    {'F','r','i'},\
    {'S','a','t'},\
};
/* ???????? */
static  const unsigned char weeksNameDutch[7][2]=
{
    {'Z','o'},
    {'M','a'},
    {'D','i'},
    {'W','o'},
    {'D','o'},
    {'V','r'},
    {'Z','a'},
};
/* ???????? */
static const unsigned char weeksNameFrench[7][3]=
{
    {'D','i','m'},
    {'L','u','n'},
    {'M','a','r'},
    {'M','e','r'},
    {'J','e','u'},
    {'V','e','n'},
    {'S','a','m'},
};
/* ???????? */
static  const unsigned char weeksNameGerman[7][2]=
{
    {'S','o'},
    {'M','o'},
    {'D','i'},
    {'M','i'},
    {'D','o'},
    {'F','r'},
    {'S','a'},
};
/* ????????? */
static  const  unsigned char weeksNameItaly[7][3]=
{
    {'D','o','m'},
    {'L','u','n'},
    {'M','a','r'},
    {'M','e','r'},
    {'G','i','o'},
    {'V','e','n'},
    {'S','a','b'},
};

/* ???????? */
static const unsigned char weeksNamePolish[7][3]=
{
    {'N','i','e'},
    {'P','o','n'},
    {'W','t','o'},
    {'S','r','o'},
    {'C','z','w'},
    {'P','i','a'},
    {'S','o','b'},
};
/* ?????????? */
static const unsigned char weeksNameSpanish[7][3]=
{
    {'D','o','m'},
    {'L','u','n'},
    {'M','a','r'},
    {'M','i','e'},
    {'J','u','e'},
    {'V','i','e'},
    {'S','a','b'},
};
/* ?????????? */
static const  unsigned char weeksNamePortugal[7][3]=
{
    {'D','o','m'},
    {'S','e','g'},
    {'T','e','r'},
    {'Q','u','a'},
    {'Q','u','i'},
    {'S','e','x'},
    {'S','a','b'},
};
/* ???????? */
static const  unsigned int weeksNameRussia[7][2]=
{
    {0xa7a3,0xa7e3},
    {0xa7b1,0xa7df},
    {0xa7a3,0xa7e4},
    {0xa7b3,0xa7e2},
    {0xa7b9,0xa7e4},
    {0xa7b1,0xa7e4},
    {0xa7b3,0xa7d2},
};
/* ????????? */
static const  unsigned int WeeksNameGreece[7][3]=
{
    {0xa6aa,0xa6b4,0xa6b1},
    {0xa6a4,0xa6a5,0xa6b4},
    {0xa6b3,0xa6b1,0xa6a9},
    {0xa6b3,0xa6a5,0xa6b3},
    {0xa6b0,0xa6a5,0xa6ac},
    {0xa6b0,0xa6a1,0xa6b1},
    {0xa6b2,0xa6a1,0xa6a2},
};


/* ???????? */
static const  unsigned int weeksNameChn[7][3]=
{
    {0xd0c7,0xc6da,0xc8d5},
    {0xd0c7,0xc6da,0xd2bb},
    {0xd0c7,0xc6da,0xb6fe},
    {0xd0c7,0xc6da,0xc8fd},
    {0xd0c7,0xc6da,0xcbc4},
    {0xd0c7,0xc6da,0xcee5},
    {0xd0c7,0xc6da,0xc1f9},
};
/* ???????? */
static const  unsigned int weeksNameJapan[7]=
{
    0x93fa,
    0x8c8e,
    0x89ce,
    0x9085,
    0x96d8,
    0x8be0,
    0x9379,
};
/*????????????*/
static const  unsigned int weeksNameHungary[7][3]=
{
    {'V','a','s'},
    {'H',0x82,'t'},
    {'K','e','d'},
    {'S','z','e'},
    {'C','s',0x81},
    {'P',0x82,'n'},
    {'S','z','o'},
};
/*?????????*/
static const  unsigned int weeksNameCzechish[7][2]=
{
    {'N','e'},
    {'P','o'},
    {0xa3,0x74},
    {'S','t'},
    {'C','t'},
    {'P',0xa0},
    {'S','o'},


};

/*??工????????*/
static const  unsigned int weeksNameSlovakian[7][2]=
{
    {'N','e'},
    {'P','o'},
    {'U','t'},
    {'S','t'},
    {'C','t'},
    {'P','i'},
    {'S','o'},

};
/*????????????*/
static const   unsigned int weeksNameTurkey[7][3]=
{
    {'P','a','z'},
    {'P','z','t'},
    {'S','a','l'},
    {'?','a','r'},
    {'P','e','r'},
    {'C','u','m'},
    {'C','t','s'},
};

static const  unsigned char monthNameEng[12][3]=
{
    {'J','a','n'},{'F','e','b'},{'M','a','r'},{'A','p','r'},{'M','a','y'},{'J','u','n'},
    {'J','u','l'},{'A','u','g'},{'S','e','p'},{'O','c','t'},{'N','o','v'},{'D','e','c'},
};
MEM_DESC text00_index_osd_buffer;
MEM_DESC text01_index_osd_buffer;
MEM_DESC text2_index_osd_buffer;
MEM_DESC pOsdBuf[2];
MEM_DESC pOsdText2Buf;
MEM_DESC pEncOsdParam;
MEM_DESC pLogoBuf;


OSD_CONFIG osdConfig;
#if 0
MASK_CONFIG  maskParam;
#endif
//static LOGO_PARAM  plogoParam;

DATE_TIME now = {2013,4,5,26,11,48,5,23};
static DATE_TIME dtOld;
static FH_UINT32    flgOsd=0;

static int OsdInitW = 0;
static int OsdInitH = 0;

static OSD_FontLib_t stChineseFontData;
static OSD_FontLib_t stAscFontData;
static FH_OSD_COLOR  stOsdColor;
static FH_OSD_COLOR  stOsdbgColor;
static FH_VPU_SIZE     stViconfig;

static rt_mutex_t   sOsdLockhandle = RT_NULL;

#define ADVOSD_LOCK rt_mutex_take(sOsdLockhandle, RT_WAITING_FOREVER)
#define ADVOSD_UNLOCK rt_mutex_release(sOsdLockhandle)

/*************************************************
  Function:        set_lun_inv
  Description:    ????OSD text????????
  input:         x ??????λ???X???
                 y ??????λ???y???
                text_scale ?????С(16*16,32*32,64*64)
                text_col_idex ?????(text0 ,text1????У?text2?????)
  output:       ?????????????????λ??0xffffffff ???λ?????????                
  Return:        ??
*************************************************/

 static void set_lun_inv(FH_UINT32 X,FH_UINT32 Y,FH_UINT32 text_scale,FH_UINT32 text_col_idex,FH_UINT32 *invert_value,FH_VPU_YCMEAN *yc_mean_info)
 {
     FH_UINT32 i,j;
     FH_UINT32 X_mocro,Y_mocro;
     FH_UINT32 lun_value = 0;
     
     if(text_scale == 16)
     {
         X_mocro = X/16;
         Y_mocro = Y/16;
         for(i = 0;i < text_col_idex;i++)
         {
             for(j = 0;j < 32;j++)
             {
                 if(*((unsigned char *)yc_mean_info->ymean.addr +((Y_mocro+i)*80+X_mocro+j)) >= OSD_INVERT_VALUE)
                 {
                     invert_value[i] |=(0x01<<j);
                 }
                 else
                 {
                     invert_value[i] &=~(0x01<<j);
                 }
             }
         }
     }
     else
     {
         if(text_scale == 32)
         {
             X_mocro=X/16;
             Y_mocro=Y/16;
             for(i = 0;i < text_col_idex;i++)
             {
                 for(j=0;j<32;j++)
                 {
                     lun_value += *((unsigned char *)yc_mean_info->ymean.addr +((Y_mocro+2*i)*80+X_mocro+2*j));
                     lun_value += *((unsigned char *)yc_mean_info->ymean.addr +((Y_mocro+2*i)*80+X_mocro+2*j+1));
                     lun_value += *((unsigned char *)yc_mean_info->ymean.addr +((Y_mocro+2*i+1)*80+X_mocro+2*j));
                     lun_value += *((unsigned char *)yc_mean_info->ymean.addr +((Y_mocro+2*i+1)*80+X_mocro+2*j+1));
 
                     //printf("aaaaaaaa%d==%d\n",32*i+j,lun_value);
                     if(lun_value >= 4*OSD_INVERT_VALUE)
                     {
                         invert_value[i] |=(0x01<<j);
                     }
                     else
                     {
                         invert_value[i] &=~(0x01<<j);
                     }     
                     lun_value = 0;
                 }
             }
         }
         else
         {
             if(text_scale == 64)
             {
                 X_mocro=X/16;
                 Y_mocro=Y/16;
                 for(i = 0;i < text_col_idex;i++)
                 {
                     for(j=0;j<32;j++)
                     {
                         lun_value += *((unsigned char *)yc_mean_info->ymean.addr +((Y_mocro+4*i)*80+X_mocro+4*j));
                         lun_value += *((unsigned char *)yc_mean_info->ymean.addr +((Y_mocro+4*i)*80+X_mocro+4*j+1));
                         lun_value += *((unsigned char *)yc_mean_info->ymean.addr +((Y_mocro+4*i)*80+X_mocro+4*j+2));
                         lun_value += *((unsigned char *)yc_mean_info->ymean.addr +((Y_mocro+4*i)*80+X_mocro+4*j+3));

                         lun_value += *((unsigned char *)yc_mean_info->ymean.addr +((Y_mocro+4*i+1)*80+X_mocro+4*j));
                         lun_value += *((unsigned char *)yc_mean_info->ymean.addr +((Y_mocro+4*i+1)*80+X_mocro+4*j+1));
                         lun_value += *((unsigned char *)yc_mean_info->ymean.addr +((Y_mocro+4*i+1)*80+X_mocro+4*j+2));
                         lun_value += *((unsigned char *)yc_mean_info->ymean.addr +((Y_mocro+4*i+1)*80+X_mocro+4*j+3));

                         lun_value += *((unsigned char *)yc_mean_info->ymean.addr +((Y_mocro+4*i+2)*80+X_mocro+4*j));
                         lun_value += *((unsigned char *)yc_mean_info->ymean.addr +((Y_mocro+4*i+2)*80+X_mocro+4*j+1));
                         lun_value += *((unsigned char *)yc_mean_info->ymean.addr +((Y_mocro+4*i+2)*80+X_mocro+4*j+2));
                         lun_value += *((unsigned char *)yc_mean_info->ymean.addr +((Y_mocro+4*i+2)*80+X_mocro+4*j+3));

                         lun_value += *((unsigned char *)yc_mean_info->ymean.addr +((Y_mocro+4*i+3)*80+X_mocro+4*j));
                         lun_value += *((unsigned char *)yc_mean_info->ymean.addr +((Y_mocro+4*i+3)*80+X_mocro+4*j+1));
                         lun_value += *((unsigned char *)yc_mean_info->ymean.addr +((Y_mocro+4*i+3)*80+X_mocro+4*j+2));
                         lun_value += *((unsigned char *)yc_mean_info->ymean.addr +((Y_mocro+4*i+3)*80+X_mocro+4*j+3));
                         
                         if(lun_value >= 16*OSD_INVERT_VALUE)
                         {
                             invert_value[i] |=(0x01<<j);
                         }
                         else
                         {
                             invert_value[i] &=~(0x01<<j);
                         }     
                         lun_value = 0;
                     }
                 }
             }
         }         
     }
}

/*************************************************
  Function:        set_invert
  Description:    ????osd???????
  Return:        ??
*************************************************/
static void set_invert()
 {
     int  ret;     
     FH_UINT32      i;
     FH_UINT32      start_x,start_y;
     FH_UINT32      amplify,font_w,text_h;
     FH_INVERT_CTRL invert_c;
     FH_VPU_OSD     OSD_INFO;
     FH_VPU_YCMEAN  yc_mean_info;
     OSD_PARAM *pParam = NULL;
     pParam = (OSD_PARAM *)pEncOsdParam.vbase;

     ret = FH_VPSS_GetYCmean(&yc_mean_info);
     if(0 != ret)
     {
         return;
     }
     invert_c.invert_enable = pParam->bTranslucent;
     for(i = 0;i < MAX_TEXT_LINE;i++)
     {
		 OSD_INFO.idex = i;
		 FH_VPSS_GetOsd(&OSD_INFO);
		 if(OSD_INFO.osd_enable == FH_FALSE)
		 {
		 	continue;
		 }
		 if(invert_c.invert_enable == 1)
		 {
			 font_w = OSD_INFO.osd_cfg.fontw;
			 text_h = OSD_INFO.osd_cfg.txth;
			 start_x = OSD_INFO.osd_startpos.u32X;
			 start_y = OSD_INFO.osd_startpos.u32Y;
			 amplify =OSD_INFO.osd_cfg.Vamp + 1;
			 font_w = amplify*font_w;
			 set_lun_inv(start_x,start_y,font_w,text_h,invert_c.invert_word,&yc_mean_info);
			 invert_c.text_idex = i;
			 FH_VPSS_SetOsdInvert(&invert_c);
		 }
		 else
		 {
			 invert_c.text_idex = i;
			 FH_VPSS_SetOsdInvert(&invert_c);
		 }
     }
}

static int init_osd(int w, int h)
{
    int i;
    int ret;
    
    MEM_DESC pVir ;
    memset(&osdConfig, 0, sizeof(osdConfig));

    OsdInitW = w;
    OsdInitH = h;
    
    //????text0 ??text1?????????????32????
    ret = buffer_malloc(&pVir, 32, DEFALT_ALIGN);
    if (ret != 0)
    {
        OSD_DBG("buffer_malloc text01_index_osd_buffer failed\n");
        return -1;
    }
    text00_index_osd_buffer.base = pVir.base;
    text00_index_osd_buffer.vbase = pVir.vbase;
    memcpy(text00_index_osd_buffer.vbase, INDEX_C, 32);

    ret = buffer_malloc(&pVir, 32, DEFALT_ALIGN);
    if (ret != 0)
    {
        OSD_DBG("buffer_malloc text01_index_osd_buffer failed\n");
        return -1;
    }
    text01_index_osd_buffer.base = pVir.base;
    text01_index_osd_buffer.vbase = pVir.vbase;
    memcpy(text01_index_osd_buffer.vbase, INDEX_C, 32);
    
    //????text2?????????????32???*8?С?
    ret = buffer_malloc(&pVir, 32 * 8, DEFALT_ALIGN);
    if (ret != 0)
    {
        OSD_DBG("buffer_malloc text2_index_osd_buffer failed\n");
        return -1;
    }
    text2_index_osd_buffer.base = pVir.base;    
    text2_index_osd_buffer.vbase = pVir.vbase;
    memcpy(text2_index_osd_buffer.vbase, INDEX_C2, 256);
    
    //????osd????
    ret = buffer_malloc(&pVir, sizeof(OSD_PARAM), DEFALT_ALIGN);
    if (ret != 0)
    {
        OSD_DBG("buffer_malloc pEncOsdParam failed\n");
        return -1;
    }
    
    pEncOsdParam.base = pVir.base;
    pEncOsdParam.vbase = pVir.vbase;
    memset((void *)pVir.vbase,0,sizeof(OSD_PARAM));

    ///????text0??text1????????(32*32*2*2)?????(16*16,32*32(16*16??????????????),
    ///64*64(32*32?????))

    ret = buffer_malloc(&pVir, 32*32*2, DEFALT_ALIGN);///??text0??text1???У????????????????
    if (ret != 0)
    {
        OSD_DBG("buffer_malloc pOsdBuf failed\n");
        return -1;
    }
    memset(pVir.vbase, 0, 32*32*2);
    for (i = 0; i < 2; i++)
    {
        pOsdBuf[i].base = pVir.base+ i*32*32;    
        pOsdBuf[i].vbase = pVir.vbase+ i*32*32;    
    }

    ///????text2????????(8*32*32*2)?????(16*16,32*32(16*16??????????????),
    ///64*64(32*32?????))

    ret = buffer_malloc(&pVir, 8*32*32, DEFALT_ALIGN);///???8??
    if (ret != 0)
    {
        OSD_DBG("buffer_malloc pOsdText2Buf failed\n");
        return -1;
    }

    pOsdText2Buf.base=pVir.base;
    pOsdText2Buf.vbase=pVir.vbase;
    memset(pVir.vbase, 0, 8*32*32);  

    ///????LOGO
    ret = buffer_malloc(&pVir, LOGO_MAX_W * LOGO_MAX_H * 4, DEFALT_ALIGN);
    if (ret != 0)
    {
        OSD_DBG("buffer_malloc pLogoBuf failed\n");
        return -1;
    }
    pLogoBuf.base=pVir.base;
    pLogoBuf.vbase=pVir.vbase;
    memset(pVir.vbase, 0, LOGO_MAX_W*LOGO_MAX_H*4); 

    ret = FH_VPSS_GetViAttr(&stViconfig);

    if (ret != 0)
    {
        OSD_DBG("[FHAdv OSD]get vi size error\n");
        return -1;
    }
    ///??????OSD???????
    FH_VPSS_EnableYCmean();

    return 0;
}


static unsigned char  shift(unsigned char data)   //??????????????
{
    data=(data<<4)|(data>>4);
    data=((data<<2)&0xcc)|((data>>2)&0x33);
    data=((data<<1)&0xaa)|((data>>1)&0x55);

    return data;
}


/*************************************************
  Function:        GET_HZ_OFFSET
  Description:    ?ж???????????
  Input:        x:??????????
  Output:        NONE
  Return:        ???????????е??????
*************************************************/
static FH_UINT32 GET_HZ_OFFSET(FH_UINT32 x)
{

        //GB2312
        return ((94*((x>>8)-0xa0-1)+((x&0xff)-0xa0-1))*32);    
        //gbk
        //return (((x&0xff)<0x80)?((190*((x>>8)-0x80-1)+((x&0xff)-0x40))*32):((190*((x>>8)-0x80-1)+((x&0xff)-0x40-1))*32));
}


/*************************************************
  Function:        GET_CHAR_TYPE
  Description:    ?ж????????
  Input:        x:    ????????
  Output:        NONE
  Return:        ???????
*************************************************/
static inline FH_UINT32 GET_CHAR_TYPE(FH_UINT32 x)
{
    if(x>=OSD_TAG_CODE_BASE && x<OSD_TAG_CODE_BASE+OSD_TAG_CNT)
    {
        return OSD_CHAR_TYPE_TAG;
    }
    else if(x<=0xff)
    {
        return OSD_CHAR_TYPE_ASC;
    }
    else if(x>=0x8000 && x<0x8080)
    {
        return OSD_CHAR_TYPE_SPECIAL;
    }
    else if((x>=0xa7a1 && x<=0xa7c1) || (x>=0xa7d1 && x<=0xa7f1))
    {
        return OSD_CHAR_TYPE_RS;
    }
    else if((x>=0xa6a1 && x<0xa6b8) || (x>=0xa6c1 && x<=0xa6d8))
    {
        return OSD_CHAR_TYPE_GREECE;
    }
    else
    {
        return OSD_CHAR_TYPE_HZ;
    }
}

/*************************************************
  Function:        GetCharFontAddr
  Description:    ?????????
  Input:        code:????????
  Output:        NONE
  Return:        ?????????
*************************************************/
static unsigned char *GetCharFontAddr(FH_UINT32 code)
{
    FH_UINT32 charType=GET_CHAR_TYPE(code);
    unsigned char* addr;

    if(charType==OSD_CHAR_TYPE_ASC)
    {
        addr=stAscFontData.pLibData+code*16;
    }
    else if(charType==OSD_CHAR_TYPE_RS)
    {
        if(code<=0xa7c1)
        {
            code-=0xa7a1;
        }
        else
        {
            code-=((0xa7d1-0xa7c1-1)+0xa7a1);
        }
        addr=stChineseFontData.pLibData+code*16;
    }
    else if(charType==OSD_CHAR_TYPE_GREECE)
    {
        if(code<=0xa6b8)
        {
            code-=0xa6a1;
        }
        else
        {
            code-=((0xa6c1-0xa6b8-1)+0xa6a1);
        }
        addr=stChineseFontData.pLibData+code*16;
    }
    else if(charType==OSD_CHAR_TYPE_SPECIAL)
    {
        code-=0x8000;
        addr=stChineseFontData.pLibData+code*16;
    }
    else
    {
        addr=stChineseFontData.pLibData+GET_HZ_OFFSET(code);
    }
    return addr;
}



/*************************************************
  Function:        UpdateTagChar
  Description:    ????????????????????????
  Input:        pTag:   ???????????
                pCode:    ??????????
  Output:        NONE
  Return:        void
*************************************************/
static void UpdateTagChar(OSD_TAG_PARAM *pTag,unsigned int *pCode,FH_UINT32 mode,FH_UINT32 chan)
{
    FH_UINT32 i;
    OSD_CHAR *pChar;
    //if(flgOsd&(1<<chan)&&(pTag->flgEncUsed&(1<<chan)))
    if(pTag->flgEncUsed&(1<<chan))
    {        
         pChar=pTag->pFirstChar[chan];
        for(i=0;i<pTag->charCnt;i++)
        {
            pChar[i].code=pCode[i];
            pChar[i].pFont=GetCharFontAddr(pCode[i]);
            //OSD_DBG("pchr_value==%d\n",pChar[i].code);
        }
        pTag->bRefresh = FH_TRUE;
    }
}
/*******************************************************************
  Function:        RefreshOsdTag
  Description:    ?????λ????????OSD????λ{????????????????}
  Input:        flg:    ??±??
  Output:        NONE
  Return:        void
*******************************************************************/
static void RefreshOsdTag(FH_UINT32 flg, FH_UINT32 mode, DATE_TIME *time, FH_UINT32 chan)
{
   FH_UINT32 code[6];
   OSD_TAG_PARAM *pOsdTag;
    
    pOsdTag = osdTagParam;
    
    if(flg&(1<<(OSD_YEAR2-OSD_TAG_CODE_BASE)))
    {
        code[0]=('0'+GET_TEN(time->year));
        code[1]=('0'+GET_ONE(time->year));
        UpdateTagChar(&pOsdTag[OSD_YEAR2-OSD_TAG_CODE_BASE],code,mode,chan);
    }
    if(flg&(1<<(OSD_YEAR4-OSD_TAG_CODE_BASE)))
    {
        code[0]=('0'+GET_THOUSAND(time->year));
        code[1]=('0'+GET_HUNDRED(time->year));
        code[2]=('0'+GET_TEN(time->year));
        code[3]=('0'+GET_ONE(time->year));
        UpdateTagChar(&pOsdTag[OSD_YEAR4-OSD_TAG_CODE_BASE],code,mode,chan);
    }
    if(flg&(1<<(OSD_MONTH2-OSD_TAG_CODE_BASE)))
    {
        code[0]=('0'+GET_TEN(time->month));
        code[1]=('0'+GET_ONE(time->month));
        UpdateTagChar(&pOsdTag[OSD_MONTH2-OSD_TAG_CODE_BASE],code,mode,chan);
    }
    if(flg&(1<<(OSD_MONTH3-OSD_TAG_CODE_BASE)))
    {
        code[0]=monthNameEng[time->month-1][0];
        code[1]=monthNameEng[time->month-1][1];
        code[2]=monthNameEng[time->month-1][2];
        UpdateTagChar(&pOsdTag[OSD_MONTH3-OSD_TAG_CODE_BASE],code,mode,chan);
    }
    if(flg&(1<<(OSD_DAY-OSD_TAG_CODE_BASE)))
    {
        code[0]=('0'+GET_TEN(time->day));
        code[1]=('0'+GET_ONE(time->day));
        UpdateTagChar(&pOsdTag[OSD_DAY-OSD_TAG_CODE_BASE],code,mode,chan);
    }

    if(flg&(1<<(OSD_WEEK3-OSD_TAG_CODE_BASE)))
    {
        code[0]=weeksNameEng[time->dayOfWeek][0];
        code[1]=weeksNameEng[time->dayOfWeek][1];
        code[2]=weeksNameEng[time->dayOfWeek][2];
        UpdateTagChar(&pOsdTag[OSD_WEEK3-OSD_TAG_CODE_BASE],code,mode,chan);
    }

    if(flg&(1<<(OSD_WEEK3_DUTCH-OSD_TAG_CODE_BASE)))
    {
        code[0]=weeksNameDutch[time->dayOfWeek][0];
        code[1]=weeksNameDutch[time->dayOfWeek][1];
        UpdateTagChar(&pOsdTag[OSD_WEEK3_DUTCH-OSD_TAG_CODE_BASE],code,mode,chan);
    }

    if(flg&(1<<(OSD_WEEK3_FRENCH-OSD_TAG_CODE_BASE)))
    {
        code[0]=weeksNameFrench[time->dayOfWeek][0];
        code[1]=weeksNameFrench[time->dayOfWeek][1];
        code[2]=weeksNameFrench[time->dayOfWeek][2];
        UpdateTagChar(&pOsdTag[OSD_WEEK3_FRENCH-OSD_TAG_CODE_BASE],code,mode,chan);
    }

    if(flg&(1<<(OSD_WEEK3_GERMAN-OSD_TAG_CODE_BASE)))
    {
        code[0]=weeksNameGerman[time->dayOfWeek][0];
        code[1]=weeksNameGerman[time->dayOfWeek][1];
        UpdateTagChar(&pOsdTag[OSD_WEEK3_GERMAN-OSD_TAG_CODE_BASE],code,mode,chan);
    }

    if(flg&(1<<(OSD_WEEK3_ITALY-OSD_TAG_CODE_BASE)))
    {
        code[0]=weeksNameItaly[time->dayOfWeek][0];
        code[1]=weeksNameItaly[time->dayOfWeek][1];
        code[2]=weeksNameItaly[time->dayOfWeek][2];
        UpdateTagChar(&pOsdTag[OSD_WEEK3_ITALY-OSD_TAG_CODE_BASE],code,mode,chan);
    }

    if(flg&(1<<(OSD_WEEK3_POLISH-OSD_TAG_CODE_BASE)))
    {
        code[0]=weeksNamePolish[time->dayOfWeek][0];
        code[1]=weeksNamePolish[time->dayOfWeek][1];
        code[2]=weeksNamePolish[time->dayOfWeek][2];
        UpdateTagChar(&pOsdTag[OSD_WEEK3_POLISH-OSD_TAG_CODE_BASE],code,mode,chan);
    }

    if(flg&(1<<(OSD_WEEK3_SPANISH-OSD_TAG_CODE_BASE)))
    {
        code[0]=weeksNameSpanish[time->dayOfWeek][0];
        code[1]=weeksNameSpanish[time->dayOfWeek][1];
        code[2]=weeksNameSpanish[time->dayOfWeek][2];
        UpdateTagChar(&pOsdTag[OSD_WEEK3_SPANISH-OSD_TAG_CODE_BASE],code,mode,chan);
    }

    if(flg&(1<<(OSD_WEEK3_PORTUGAL-OSD_TAG_CODE_BASE)))
    {
        code[0]=weeksNamePortugal[time->dayOfWeek][0];
        code[1]=weeksNamePortugal[time->dayOfWeek][1];
        code[2]=weeksNamePortugal[time->dayOfWeek][2];
        UpdateTagChar(&pOsdTag[OSD_WEEK3_PORTUGAL-OSD_TAG_CODE_BASE],code,mode,chan);
    }

    if(flg&(1<<(OSD_WEEK3_RUSSIA-OSD_TAG_CODE_BASE)))
    {
        code[0]=weeksNameRussia[time->dayOfWeek][0];
        code[1]=weeksNameRussia[time->dayOfWeek][1];
        UpdateTagChar(&pOsdTag[OSD_WEEK3_RUSSIA-OSD_TAG_CODE_BASE],code,mode,chan);
    }

    if(flg&(1<<(OSD_WEEK3_GREECE-OSD_TAG_CODE_BASE)))
    {
        code[0]=WeeksNameGreece[time->dayOfWeek][0];
        code[1]=WeeksNameGreece[time->dayOfWeek][1];
        code[2]=WeeksNameGreece[time->dayOfWeek][2];
        UpdateTagChar(&pOsdTag[OSD_WEEK3_GREECE-OSD_TAG_CODE_BASE],code,mode,chan);
    }

    if(flg&(1<<(OSD_CWEEK1-OSD_TAG_CODE_BASE)))
    {
        code[0]=weeksNameChn[time->dayOfWeek][0];
        code[1]=weeksNameChn[time->dayOfWeek][1];
        code[2]=weeksNameChn[time->dayOfWeek][2];
        UpdateTagChar(&pOsdTag[OSD_CWEEK1-OSD_TAG_CODE_BASE],code,mode,chan);
    }

    if(flg&(1<<(OSD_CWEEK1_JAPAN-OSD_TAG_CODE_BASE)))
    {
        code[0]=weeksNameJapan[time->dayOfWeek];
        UpdateTagChar(&pOsdTag[OSD_CWEEK1_JAPAN-OSD_TAG_CODE_BASE],code,mode,chan);
    }

    if(flg&(1<<(OSD_WEEK3_HUNGARY-OSD_TAG_CODE_BASE)))
    {
        code[0]=weeksNameHungary[time->dayOfWeek][0];
        code[1]=weeksNameHungary[time->dayOfWeek][1];
        code[2]=weeksNameHungary[time->dayOfWeek][2];
        UpdateTagChar(&pOsdTag[OSD_WEEK3_HUNGARY-OSD_TAG_CODE_BASE],code,mode,chan);
    }

    if(flg&(1<<(OSD_WEEK3_CZECHISH-OSD_TAG_CODE_BASE)))
    {
        code[0]=weeksNameCzechish[time->dayOfWeek][0];
        code[1]=weeksNameCzechish[time->dayOfWeek][1];
        UpdateTagChar(&pOsdTag[OSD_WEEK3_CZECHISH-OSD_TAG_CODE_BASE],code,mode,chan);
    }

    if(flg&(1<<(OSD_WEEK3_SLOVAKIAN-OSD_TAG_CODE_BASE)))
    {
        code[0]=weeksNameSlovakian[time->dayOfWeek][0];
        code[1]=weeksNameSlovakian[time->dayOfWeek][1];
        UpdateTagChar(&pOsdTag[OSD_WEEK3_SLOVAKIAN-OSD_TAG_CODE_BASE],code,mode,chan);
    }

    if(flg&(1<<(OSD_WEEK3_TURKEY-OSD_TAG_CODE_BASE)))
    {
        code[0]=weeksNameTurkey[time->dayOfWeek][0];
        code[1]=weeksNameTurkey[time->dayOfWeek][1];
        code[2]=weeksNameTurkey[time->dayOfWeek][2];
        UpdateTagChar(&pOsdTag[OSD_WEEK3_TURKEY-OSD_TAG_CODE_BASE],code,mode,chan);
    }
    if(flg&(1<<(OSD_HOUR24-OSD_TAG_CODE_BASE)))
    {
        code[0]=('0'+GET_TEN(time->hour));
        code[1]=('0'+GET_ONE(time->hour));
        UpdateTagChar(&pOsdTag[OSD_HOUR24-OSD_TAG_CODE_BASE],code,mode,chan);
    }
    if(flg&(1<<(OSD_HOUR12-OSD_TAG_CODE_BASE)))
    {
        if(time->hour>12)
        {
            code[0]=('0'+GET_TEN(time->hour-12));
            code[1]=('0'+GET_ONE(time->hour-12));
        }
        else
        {
            code[0]=('0'+GET_TEN(time->hour));
            code[1]=('0'+GET_ONE(time->hour));
        }
        UpdateTagChar(&pOsdTag[OSD_HOUR12-OSD_TAG_CODE_BASE],code,mode,chan);
    }
    if(flg&(1<<(OSD_MINUTE-OSD_TAG_CODE_BASE)))
    {
        code[0]=('0'+GET_TEN(time->minute));
        code[1]=('0'+GET_ONE(time->minute));
        UpdateTagChar(&pOsdTag[OSD_MINUTE-OSD_TAG_CODE_BASE],code,mode,chan);
    }
    if(flg&(1<<(OSD_SECOND-OSD_TAG_CODE_BASE)))
    {
        code[0]=('0'+GET_TEN(time->second));
        code[1]=('0'+GET_ONE(time->second));
#ifdef MILLISEC_OSD
        code[2]=':';
        code[3]=('0'+GET_HUNDRED(time->milliSecond));
        code[4]=('0'+GET_TEN(time->milliSecond));
#endif
        UpdateTagChar(&pOsdTag[OSD_SECOND-OSD_TAG_CODE_BASE],code,mode,chan);
    }
    if(flg&(1<<(OSD_AMPM-OSD_TAG_CODE_BASE)))
    {    
        if(time->hour<12)
        {
            code[0]='A';
        }
        else
        {
            code[0]='P';
        }
        code[1]='M';
        UpdateTagChar(&pOsdTag[OSD_AMPM-OSD_TAG_CODE_BASE],code,mode,chan);
    }
}

/********************************************************
  Function:        RefreshFlgYear
  Description:    ???????
  Input:        pFlgEnc:  ???????OSD??????
                pFlgDec:  ???????OSD??????
                pFlgDisp: ??????OSD??????
  Output:        NONE
  Return:        void
*********************************************************/
static void RefreshFlgYear(FH_UINT32 *pFlgEnc)
{
    if(osdTagParam[OSD_YEAR2-OSD_TAG_CODE_BASE].flgEncUsed)
    {
        *pFlgEnc|=(1<<(OSD_YEAR2-OSD_TAG_CODE_BASE));
    }
    if(osdTagParam[OSD_YEAR4-OSD_TAG_CODE_BASE].flgEncUsed)
    {
        *pFlgEnc|=(1<<(OSD_YEAR4-OSD_TAG_CODE_BASE));
    }
}

/********************************************************
  Function:        RefreshFlgMonth
  Description:    ????±??
  Input:        pFlgEnc:  ???????OSD??????
                pFlgDec:  ???????OSD??????
                pFlgDisp: ??????OSD??????
  Output:        NONE
  Return:        void
*********************************************************/
static void RefreshFlgMonth(FH_UINT32 *pFlgEnc)
{
    if(osdTagParam[OSD_MONTH2-OSD_TAG_CODE_BASE].flgEncUsed)
    {
        *pFlgEnc|=(1<<(OSD_MONTH2-OSD_TAG_CODE_BASE));
    }
    if(osdTagParam[OSD_MONTH3-OSD_TAG_CODE_BASE].flgEncUsed)
    {
        *pFlgEnc|=(1<<(OSD_MONTH3-OSD_TAG_CODE_BASE));
    }
}

/********************************************************
  Function:        RefreshDay
  Description:    ???????
  Input:        pFlgEnc:  ???????OSD??????
                pFlgDec:  ???????OSD??????
                pFlgDisp: ??????OSD??????
  Output:        NONE
  Return:        void
*********************************************************/
static void RefreshDay(FH_UINT32 *pFlgEnc)
{
    if(osdTagParam[OSD_DAY-OSD_TAG_CODE_BASE].flgEncUsed)
    {
        *pFlgEnc|=(1<<(OSD_DAY-OSD_TAG_CODE_BASE));
    }
}

/********************************************************
  Function:        RefreshWeek
  Description:    ?????????
  Input:        pFlgEnc:  ???????OSD??????
                pFlgDec:  ???????OSD??????
                pFlgDisp: ??????OSD??????
  Output:        NONE
  Return:        void
*********************************************************/
static void RefreshWeek(FH_UINT32 *pFlgEnc)
{
    if(osdTagParam[OSD_WEEK3-OSD_TAG_CODE_BASE].flgEncUsed)
    {
        *pFlgEnc|=(1<<(OSD_WEEK3-OSD_TAG_CODE_BASE));
    }
    if(osdTagParam[OSD_CWEEK1-OSD_TAG_CODE_BASE].flgEncUsed)
    {
        *pFlgEnc|=(1<<(OSD_CWEEK1-OSD_TAG_CODE_BASE));
    }
    if(osdTagParam[OSD_WEEK3_DUTCH-OSD_TAG_CODE_BASE].flgEncUsed)
    {
        *pFlgEnc|=(1<<(OSD_WEEK3_DUTCH-OSD_TAG_CODE_BASE));
    }
    if(osdTagParam[OSD_WEEK3_FRENCH-OSD_TAG_CODE_BASE].flgEncUsed)
    {
        *pFlgEnc|=(1<<(OSD_WEEK3_FRENCH-OSD_TAG_CODE_BASE));
    }
    if(osdTagParam[OSD_WEEK3_GERMAN-OSD_TAG_CODE_BASE].flgEncUsed)
    {
        *pFlgEnc|=(1<<(OSD_WEEK3_GERMAN-OSD_TAG_CODE_BASE));
    }
    if(osdTagParam[OSD_WEEK3_ITALY-OSD_TAG_CODE_BASE].flgEncUsed)
    {
        *pFlgEnc|=(1<<(OSD_WEEK3_ITALY-OSD_TAG_CODE_BASE));
    }
    if(osdTagParam[OSD_WEEK3_POLISH-OSD_TAG_CODE_BASE].flgEncUsed)
    {
        *pFlgEnc|=(1<<(OSD_WEEK3_POLISH-OSD_TAG_CODE_BASE));
    }
    if(osdTagParam[OSD_WEEK3_SPANISH-OSD_TAG_CODE_BASE].flgEncUsed)
    {
        *pFlgEnc|=(1<<(OSD_WEEK3_SPANISH-OSD_TAG_CODE_BASE));
    }
    if(osdTagParam[OSD_WEEK3_PORTUGAL-OSD_TAG_CODE_BASE].flgEncUsed)
    {
        *pFlgEnc|=(1<<(OSD_WEEK3_PORTUGAL-OSD_TAG_CODE_BASE));
    }
    if(osdTagParam[OSD_WEEK3_RUSSIA-OSD_TAG_CODE_BASE].flgEncUsed)
    {
        *pFlgEnc|=(1<<(OSD_WEEK3_RUSSIA-OSD_TAG_CODE_BASE));
    }
    if(osdTagParam[OSD_CWEEK1_JAPAN-OSD_TAG_CODE_BASE].flgEncUsed)
    {
        *pFlgEnc|=(1<<(OSD_CWEEK1_JAPAN-OSD_TAG_CODE_BASE));
    }
    if(osdTagParam[OSD_WEEK3_HUNGARY-OSD_TAG_CODE_BASE].flgEncUsed)
    {
        *pFlgEnc|=(1<<(OSD_WEEK3_HUNGARY-OSD_TAG_CODE_BASE));
    }
    if(osdTagParam[OSD_WEEK3_CZECHISH-OSD_TAG_CODE_BASE].flgEncUsed)
    {
        *pFlgEnc|=(1<<(OSD_WEEK3_CZECHISH-OSD_TAG_CODE_BASE));
    }
    if(osdTagParam[OSD_WEEK3_SLOVAKIAN-OSD_TAG_CODE_BASE].flgEncUsed)
    {
        *pFlgEnc|=(1<<(OSD_WEEK3_SLOVAKIAN-OSD_TAG_CODE_BASE));
    }
    if(osdTagParam[OSD_WEEK3_TURKEY-OSD_TAG_CODE_BASE].flgEncUsed)
    {
        *pFlgEnc|=(1<<(OSD_WEEK3_TURKEY-OSD_TAG_CODE_BASE));
    }
}

/********************************************************
  Function:        RefreshHour
  Description:    ???С????
  Input:        pFlgEnc:  ???????OSD??????
                pFlgDec:  ???????OSD??????
                pFlgDisp: ??????OSD??????
  Output:        NONE
  Return:        void
*********************************************************/


static void RefreshHour(FH_UINT32 *pFlgEnc)
{
    if(osdTagParam[OSD_HOUR24-OSD_TAG_CODE_BASE].flgEncUsed)
    {
        *pFlgEnc|=(1<<(OSD_HOUR24-OSD_TAG_CODE_BASE));
    }
    if(osdTagParam[OSD_HOUR12-OSD_TAG_CODE_BASE].flgEncUsed)
    {
        *pFlgEnc|=(1<<(OSD_HOUR12-OSD_TAG_CODE_BASE));
    }
}

/********************************************************
  Function:        RefreshMinute
  Description:    ??·?????
  Input:        pFlgEnc:  ???????OSD??????
                pFlgDec:  ???????OSD??????
                pFlgDisp: ??????OSD??????
  Output:        NONE
  Return:        void
*********************************************************/
static void RefreshMinute(FH_UINT32 *pFlgEnc)
{
    if(osdTagParam[OSD_MINUTE-OSD_TAG_CODE_BASE].flgEncUsed)
    {
        *pFlgEnc|=(1<<(OSD_MINUTE-OSD_TAG_CODE_BASE));
    }
}

/********************************************************
  Function:        RefreshSecond
  Description:    ???????
  Input:        pFlgEnc:  ???????OSD??????
                pFlgDec:  ???????OSD??????
                pFlgDisp: ??????OSD??????
  Output:        NONE
  Return:        void
*********************************************************/
static void RefreshSecond(FH_UINT32 *pFlgEnc)
{
    if(osdTagParam[OSD_SECOND-OSD_TAG_CODE_BASE].flgEncUsed)
    {
        *pFlgEnc|=(1<<(OSD_SECOND-OSD_TAG_CODE_BASE));
    }
}




/**
*     RefreshOsdParamTag
*\brief    
*\param[in]     FH_UINT32 chan       
*\param[in]     FH_UINT32 mode       
*\param[in]     UINT32 osdType  
*\param[out] void
*\return void
*/
static void RefreshOsdParamTag(FH_UINT32 chan, OSD_TAG_PARAM *pTagParam,FH_UINT32 mode)
{
    FH_UINT32 osdCnt = 0, w, h, line = 0,x,y;
    OSD_CHAR *pChar = NULL;
    OSD_PARAM    *pParam=NULL;
    OSD_LINE  *pLine;
    FH_UINT32 LUN_VALUE;
    FH_VPU_OSD  vpu_osd_info;
    unsigned char amplify;
    pParam = (OSD_PARAM *)pEncOsdParam.vbase;
    pChar = pTagParam->pFirstChar[chan];   
    FH_UINT32 char_size = 0;
    FH_UINT32 all_char_size = 0;
    pLine = pParam->line ;

    float ratio;
     ratio = ((float)stViconfig.vi_size.u32Height/(float)stViconfig.vi_size.u32Width);
    if (pChar == NULL)
    {
        OSD_DBG("pChar ERRRR\n");
        return;
    }

      if(!flgOsd )
      {
        return;
    }
        
    for (osdCnt = 0; osdCnt < pTagParam->charCnt; osdCnt++)
    {
        w=pChar[osdCnt].size&0xff;
        h=(pChar[osdCnt].size>>8)&0xff;
        y = (pChar->pos >> 16);
        x = (pChar->pos & 0xffff);        
        line = pTagParam->line;
        if (pParam->line[line].pDrawFunc == NULL)
        {    
            break;
        }
        x = x + osdCnt*w;

        pParam->line[line].pDrawFunc(pChar[osdCnt].pFont, pOsdBuf[line].vbase + 2*x, w, h, pParam->line[line].w,pChar->charType);
        pChar[osdCnt].bChanged = FH_TRUE;
    }

    if ((pLine->osdScaleH == 0) && (pLine->osdScaleV == 0))  
    {
        vpu_osd_info.osd_cfg.fontw = 8;
        vpu_osd_info.osd_cfg.fonth = 16;
        vpu_osd_info.osd_cfg.Vamp =  0;
        vpu_osd_info.osd_cfg.Hamp = 0;
        amplify = 8;
    }
    else  if ((pLine->osdScaleH == 0) && (pLine->osdScaleV == 1))
    {
        vpu_osd_info.osd_cfg.fontw = 8;
        vpu_osd_info.osd_cfg.fonth = 16;
        vpu_osd_info.osd_cfg.Vamp =  1;
        vpu_osd_info.osd_cfg.Hamp = 1;
        amplify = 16;
    }
    else if((pLine->osdScaleH == 1) && (pLine->osdScaleV == 1))
    {
        vpu_osd_info.osd_cfg.fontw = 16;
        vpu_osd_info.osd_cfg.fonth = 32;
        vpu_osd_info.osd_cfg.Vamp =  1;
        vpu_osd_info.osd_cfg.Hamp = 1;
        amplify = 32;
    }
    else
    {
        vpu_osd_info.osd_cfg.fontw = 8;
        vpu_osd_info.osd_cfg.fonth = 16;
        vpu_osd_info.osd_cfg.Vamp =  0;
        vpu_osd_info.osd_cfg.Hamp = 0;
        amplify = 8;
    }


    char_size = amplify;
    all_char_size = pLine->validCharCnt * char_size;
    if(rotate_value == 1)
    {
        vpu_osd_info.osd_cfg.txtw = pLine->validCharCnt;
        memcpy(text01_index_osd_buffer.vbase, INDEX_C_ROTATE +(32 -pLine->validCharCnt), pLine->validCharCnt);
        x = pLine->y;

        if(stViconfig.vi_size.u32Height >= ((pLine->x *(ratio))+all_char_size))
            y = stViconfig.vi_size.u32Height-(pLine->x *(ratio))-all_char_size;
        else
            y = 0;
    }
    else if(rotate_value == 2)
    {
        vpu_osd_info.osd_cfg.txtw = pLine->validCharCnt;
        memcpy(text01_index_osd_buffer.vbase, INDEX_C_ROTATE+(32 -pLine->validCharCnt), pLine->validCharCnt);

        if(stViconfig.vi_size.u32Width >= (all_char_size+pLine->x))
            x = stViconfig.vi_size.u32Width-all_char_size-pLine->x;
        else
            x = 0;

        if(stViconfig.vi_size.u32Height >= (2*char_size+pLine->y))
            y = stViconfig.vi_size.u32Height-2*char_size-pLine->y;
        else
            y = 0;
    }
    else if(rotate_value == 3)
    {
        vpu_osd_info.osd_cfg.txtw = pLine->validCharCnt;
        memcpy(text01_index_osd_buffer.vbase, INDEX_C, 32);

        if(stViconfig.vi_size.u32Width >= (2*char_size+pLine->y))
            x = stViconfig.vi_size.u32Width-2*char_size-pLine->y;
        else
            x =0;
        y = pLine->x* ratio;
    }
    else
    {
        vpu_osd_info.osd_cfg.txtw =pLine->validCharCnt;
        memcpy(text01_index_osd_buffer.vbase, INDEX_C, 32);
        x = pLine->x;
        y = pLine->y;
    }

    LUN_VALUE = pLine->LINE_COLOR;
    vpu_osd_info.idex = 0;
    vpu_osd_info.osd_enable = 1;
    vpu_osd_info.osdfontaddr = (unsigned char *)pOsdBuf[0].base;
    vpu_osd_info.osdtxtaddr = (unsigned char *)text01_index_osd_buffer.base;
    vpu_osd_info.osd_cfg.txth = 1;    

    vpu_osd_info.osd_color.norcolor = stOsdColor.norcolor;//0xf55e;
    vpu_osd_info.osd_color.invcolor = stOsdColor.invcolor;//0xf000;
    vpu_osd_info.osd_blgcolor.norcolor = stOsdbgColor.norcolor;
    vpu_osd_info.osd_blgcolor.invcolor = stOsdbgColor.invcolor;
    if(rotate_flag == 1)
    {
        vpu_osd_info.osd_cfg.txtrotate = rotate_value;
    }
    else
    {
        vpu_osd_info.osd_cfg.txtrotate = 0;
    }

    vpu_osd_info.osd_startpos.u32X= x;
    vpu_osd_info.osd_startpos.u32Y= y; 

    FH_VPSS_SetOsd(&vpu_osd_info);
}

/********************************************************
  Function:        RefreshAMPM
  Description:    ???AMPM???
  Input:        pFlgEnc:  ???????OSD??????
                pFlgDec:  ???????OSD??????
                pFlgDisp: ??????OSD??????
  Output:        NONE
  Return:        void
*********************************************************/
static void RefreshAMPM(FH_UINT32 *pFlgEnc)
{
    if(osdTagParam[OSD_AMPM-OSD_TAG_CODE_BASE].flgEncUsed)
    {
        *pFlgEnc|=(1<<(OSD_AMPM-OSD_TAG_CODE_BASE));
    }
}

static void _updateNowtime(void)
{
    SYSTEM_TIME_T _curtimeymd;
    
    GetCurrentSystemTime(&_curtimeymd);
    now.milliSecond=0;
    now.second = _curtimeymd.second;             
    now.minute = _curtimeymd.minute;            
    now.hour = _curtimeymd.hour;            
    now.dayOfWeek = _curtimeymd.wday;            
    now.day = _curtimeymd.day;            
    now.month = _curtimeymd.month;        
    now.year = _curtimeymd.year;
}
static FH_UINT32 RefreshDateTime(void)
{
    FH_UINT32 FlgEnc = 0;
    //FH_UINT32 FlgDisp = 0;
    FH_UINT32 i,j;
    
    _updateNowtime();

    
    if (dtOld.year != now.year)
    {
        /* ??????? */
        RefreshFlgYear(&FlgEnc);
    }
    
    if (dtOld.month != now.month)
    {
        /* ????±?? */
        RefreshFlgMonth(&FlgEnc);
    }
    
    if (dtOld.dayOfWeek != now.dayOfWeek)
    {
        /* ??????? */
        RefreshWeek(&FlgEnc);
    }
    
    if (dtOld.day != now.day)
    {
        /* ??????? */
        RefreshDay(&FlgEnc);
    }
    
    if (dtOld.hour != now.hour)
    {
        /* ???С???? */
        RefreshHour(&FlgEnc);
        /* ???AMPM??? */
        RefreshAMPM(&FlgEnc);
    }
    
    if (dtOld.minute != now.minute)
    {
        /* ??·????? */
        RefreshMinute(&FlgEnc);
    }
    
    if (dtOld.second != now.second)
    {
        /* ??????? */
        RefreshSecond(&FlgEnc);
    }
    
    memcpy(&dtOld, (const void *)(&now), sizeof(DATE_TIME));
    ADVOSD_LOCK;
    for (i = 0; i < 1; i++)
    {
        if (flgOsd & (1 << i))
        {
            RefreshOsdTag(FlgEnc, 0, (DATE_TIME *)(&now),i);
            for (j = 0; j < OSD_TAG_CNT; j++)
            {
                if (osdTagParam[j].bRefresh)
                {
                    osdTagParam[j].bRefresh = FH_FALSE;
                    RefreshOsdParamTag(i,&osdTagParam[j], 0);
                }                
            }
        }
    }
    ADVOSD_UNLOCK;
    return FlgEnc;
}

static void char_invert(unsigned char *new_char,unsigned char *old_char)
{
    int i;
    for(i = 0;i < 256;i++)
    {
        new_char[i] = old_char[255 -i];
    }
}

static void display_chars(unsigned int *x, unsigned int *y)
{
    FH_UINT32 i;
    FH_UINT32 temp;
    OSD_PARAM *pParam = NULL;
    OSD_LINE *pLine;
    unsigned char w, h;
    unsigned char amplify;
    FH_VPU_OSD vpu_osd_info;
    FH_UINT32 all_char_size;
    FH_UINT32 char_size;
    float   ratio;

    pParam = (OSD_PARAM *)pEncOsdParam.vbase;

    ratio = ((float)stViconfig.vi_size.u32Height/(float)stViconfig.vi_size.u32Width);

    for (i = 0; i < 3; i++)
    {
        pLine = pParam->line +i;

        if ((pLine->osdScaleH == 0) && (pLine->osdScaleV == 0))  
        {
            w = 8, h = 16;
            amplify = 0;
        }
        else if ((pLine->osdScaleH == 0) && (pLine->osdScaleV == 1))
        {
            w = 8, h = 16;
            amplify = 1;
        }
        else if ((pLine->osdScaleH == 1) && (pLine->osdScaleV == 1))
        {
            w = 16, h = 32;
            amplify = 1;
        }
        else
        {
            w = 8, h = 16;
            amplify = 0;
        }

        vpu_osd_info.idex = i;

        if (pLine->charCnt ==0)
            vpu_osd_info.osd_enable= 0;
        else
            vpu_osd_info.osd_enable= 1;
        char_size = w * (amplify+1);

        
        if (i == 2)
        {
            all_char_size = 32 * char_size;
            if(rotate_value == 1)
            {
                {
                    temp = y[i];
                    if(stViconfig.vi_size.u32Height >= (all_char_size+(x[i]*ratio)))
                        y[i] = stViconfig.vi_size.u32Height -all_char_size-(x[i]*ratio);
                    else
                        y[i] = 0;
                    x[i] = temp/ratio;
                    
                    memcpy(text2_index_osd_buffer.vbase, INDEX_C2_90+(256 -8*pLine->validCharCnt), 8*pLine->validCharCnt);
                    vpu_osd_info.osd_cfg.txtw = pLine->validCharCnt;
                }    
            }
            else if(rotate_value == 2)
            {
                char_invert(INDEX_C2_180,INDEX_C2);
                memcpy(text2_index_osd_buffer.vbase, INDEX_C2_180+(256 -8*pLine->validCharCnt), 8*pLine->validCharCnt);

                if(stViconfig.vi_size.u32Width >= (all_char_size+x[i]))
                    x[i] = stViconfig.vi_size.u32Width -all_char_size-x[i];
                else
                    x[i] = 0;
                
                if(stViconfig.vi_size.u32Height >= (h*(amplify+1)*8+y[i]))
                    y[i] = stViconfig.vi_size.u32Height-h*(amplify+1)*8-y[i];
                else
                    y[i] = 0;
                
                vpu_osd_info.osd_cfg.txtw = pLine->validCharCnt;
            }
            else if(rotate_value == 3)
            {
                char_invert(INDEX_C2_270,INDEX_C2_90);
                memcpy(text2_index_osd_buffer.vbase, INDEX_C2_270, 256);
                temp = x[i];

                if(stViconfig.vi_size.u32Width >= (h*(amplify+1)*8+y[i]))
                    x[i] = (stViconfig.vi_size.u32Width-h*(amplify+1)*8-y[i]);
                else
                    x[i] = 0;
                
                y[i] = temp*ratio;
                vpu_osd_info.osd_cfg.txtw = pLine->validCharCnt;
            }
            else
            {
                vpu_osd_info.osd_cfg.txtw = pLine->validCharCnt;
                memcpy(text2_index_osd_buffer.vbase, INDEX_C2, 256);
                vpu_osd_info.osd_cfg.txtrotate = 0;
            }
                
            vpu_osd_info.osd_cfg.txtrotate = rotate_value;
            vpu_osd_info.osdfontaddr = (unsigned char *)pOsdText2Buf.base;
            vpu_osd_info.osdtxtaddr = (unsigned char *)text2_index_osd_buffer.base;
            if((pLine->occupyCharCnt/32)<1)
                vpu_osd_info.osd_cfg.txth = 1;
            else
                vpu_osd_info.osd_cfg.txth = pLine->validCharCnt/32;

            vpu_osd_info.osd_cfg.txth = 8;
        }
        else
        {
            all_char_size = pLine->validCharCnt * char_size;
            if(rotate_value == 1)
            {
                vpu_osd_info.osd_cfg.txtw = pLine->validCharCnt;
                memcpy(text00_index_osd_buffer.vbase, INDEX_C_ROTATE +(32 -pLine->validCharCnt), pLine->validCharCnt);
                temp = x[i];
                x[i] = y[i];
                
                if(stViconfig.vi_size.u32Height >=((temp*ratio)+all_char_size))
                    y[i] = stViconfig.vi_size.u32Height-(temp*ratio)-all_char_size;
                else
                    y[i] = 0;
            }
            else if(rotate_value == 2)
            {
                memcpy(text00_index_osd_buffer.vbase, INDEX_C_ROTATE+(32 -pLine->validCharCnt), pLine->validCharCnt);
                if(stViconfig.vi_size.u32Width >= (all_char_size+x[i]))
                    x[i] = stViconfig.vi_size.u32Width-all_char_size-x[i];
                else
                    x[i] = 0;

                if(stViconfig.vi_size.u32Height>= (h*(amplify+1)+y[i]))
                    y[i] = stViconfig.vi_size.u32Height-h*(amplify+1)-y[i];
                else
                    y[i] = 0;
                vpu_osd_info.osd_cfg.txtw = pLine->validCharCnt;
            }
            else if(rotate_value == 3)
            {
                memcpy(text00_index_osd_buffer.vbase, INDEX_C, 32);
                temp = x[i];
                if(stViconfig.vi_size.u32Width>= (h*(amplify+1)+y[i]))
                    x[i] = stViconfig.vi_size.u32Width-h*(amplify+1)-y[i];
                else
                    x[i] = 0;
                y[i] = temp* ratio;
                vpu_osd_info.osd_cfg.txtw = pLine->validCharCnt;
            }
            else
            {
                vpu_osd_info.osd_cfg.txtw = pLine->validCharCnt;
                memcpy(text00_index_osd_buffer.vbase, INDEX_C, 32);
            }
            vpu_osd_info.osd_cfg.txtrotate = rotate_value;
            vpu_osd_info.osdfontaddr = (unsigned char *)pOsdBuf[i].base;
            vpu_osd_info.osdtxtaddr = (unsigned char *)text00_index_osd_buffer.base;
            vpu_osd_info.osd_cfg.txth = 1;
        }

        vpu_osd_info.osd_color.norcolor = stOsdColor.norcolor;
        vpu_osd_info.osd_color.invcolor = stOsdColor.invcolor;
        vpu_osd_info.osd_blgcolor.norcolor = stOsdbgColor.norcolor;
        vpu_osd_info.osd_blgcolor.invcolor = stOsdbgColor.invcolor;

        if (amplify == 1)
        {
            vpu_osd_info.osd_cfg.Vamp = 1;
            vpu_osd_info.osd_cfg.Hamp = 1;
        }
        else
        {
            vpu_osd_info.osd_cfg.Vamp = 0;
            vpu_osd_info.osd_cfg.Hamp = 0;
        }

        vpu_osd_info.osd_cfg.fontw = w;
        vpu_osd_info.osd_cfg.fonth = h;
        vpu_osd_info.osd_startpos.u32X= x[i];
        vpu_osd_info.osd_startpos.u32Y= y[i];
        if(rotate_flag == 1)
        {
            vpu_osd_info.osd_cfg.txtrotate = rotate_value;
        }
        else
        {
            vpu_osd_info.osd_cfg.txtrotate = 0;
        }

        FH_VPSS_SetOsd(&vpu_osd_info);
    }
}

static void draw_osd(FH_UINT32 chan, FH_UINT32 mode)
{
    OSD_PARAM *pParam = NULL;
    OSD_LINE *pLine;
    OSD_CHAR *pChar;
    FH_UINT32 i, j, w, h, x, y;
    FH_UINT32 x_pos[OSD_MAX_LINE_BASE];
    FH_UINT32 y_pos[OSD_MAX_LINE_BASE];
    chan = 0;

    int encMaxWidth = OsdInitW;    // ???? OSD ????? OSD ???????
    int osdLineMaxWidth = 0;
    
    pParam = (OSD_PARAM *)pEncOsdParam.vbase;

    if (pParam->bReady == FH_FALSE)
    {
        return;
    }

    //printf("WIDTH==%d\n",encMaxWidth);
    if (encMaxWidth > 32 * 32)
    {
        osdLineMaxWidth = 32 * 32;
    }
    else
    {
        osdLineMaxWidth = encMaxWidth;
    }
    memset(pOsdText2Buf.vbase, 0, 4*32*32);

    for (i = 0; i < OSD_MAX_LINE_BASE; i++)
    {    
        if (i >= MAX_OSD_REGION)
        {
            continue;
        }
        pLine = pParam->line + i;
        if(i <3)
        {
            x_pos[i] = pLine->x;
            y_pos[i] = pLine->y;
        }
        if (pLine->validCharCnt == 0)
        {
            continue;
        }  
       
        if(i < 2)
        {
            memset(pOsdBuf[i].vbase, 0, 32 * 32);
            for (j = 0; j < pLine->validCharCnt; j++)
            {
                pChar = pLine->charData + j;
                y = pChar->pos >> 16;
                x = pChar->pos & 0xffff;        
                w = pChar->size & 0xff;
                h = (pChar->size >> 8) & 0xff;
                pLine->pDrawFunc(pChar->pFont, pOsdBuf[i].vbase + 2*x , w, h, pLine->w, pChar->charType);
                pChar->bChanged = FH_TRUE;
            }
        }
        else
        {
            for (j = 0; j < pLine->validCharCnt; j++)
            {
                pChar = pLine->charData + j;
                y = pChar->pos >> 16;
                x = pChar->pos & 0xffff;        
                w = pChar->size & 0xff;
                h = (pChar->size >> 8) & 0xff;
                pLine->pDrawFunc(pChar->pFont, pOsdText2Buf.vbase + 2*x  + 16*32 * (i - 2), w, h, pLine->w, pChar->charType);        
                pChar->bChanged = FH_TRUE;
            }
        }
    }
    display_chars(x_pos, y_pos);
}

static void DrawCharH1V1(unsigned char *pFont, unsigned char *pDst, FH_UINT32 w, FH_UINT32 h, FH_UINT32 pitch, FH_UINT32 chartype)
{
    FH_UINT32 i;
    unsigned char *pTemp = pDst;

    if (chartype == OSD_CHAR_TYPE_HZ || (chartype == OSD_CHAR_TYPE_TAG && pFont >= stChineseFontData.pLibData && pFont <= stChineseFontData.pLibData + 267616))
    {
        for (i = 0; i < 8; i++)
        {
            pTemp[i] = shift(pFont[2*i]);
            pTemp[i+8] = shift(pFont[2*i+16]);
            pTemp[i+16] = shift(pFont[2*i+1]);
            pTemp[i+24] = shift(pFont[2*i+17]);
        }
    }
    else
    {
        for (i = 0; i < 8; i++)
        {
            pTemp[i] = shift(pFont[i]) ;
            //pTemp[i+8] = 0 ;
            pTemp[i+8] = shift(pFont[i+8]) ;
        //pTemp[i+24] = 0;
        }
    }
}

static void DrawCharH2V2(unsigned char *pFont, unsigned char *pDst, FH_UINT32 w, FH_UINT32 h, FH_UINT32 pitch, FH_UINT32 chartype)
{
    FH_UINT32 i;
    unsigned char *pTemp = pDst;
    
    if (chartype == OSD_CHAR_TYPE_HZ || (chartype == OSD_CHAR_TYPE_TAG && pFont >= stChineseFontData.pLibData && pFont <= stChineseFontData.pLibData + 267616))
    {
        for (i = 0; i < 8; i++)
        {
            pTemp[i+8] = pTemp[i] = shift(pFont[2*i]);
            pTemp[i+40] = pTemp[i+32] = shift(pFont[2*i]);
            pTemp[i+24] = pTemp[i+16] = shift(pFont[2*i+1]);
            pTemp[i+56] = pTemp[i+48] = shift(pFont[2*i+1]);
            pTemp[i+72] = pTemp[i+64] = shift(pFont[2*i+16]);
            pTemp[i+104] = pTemp[i+96] = shift(pFont[2*i+16]);
            pTemp[i+88] = pTemp[i+82] = shift(pFont[2*i+17]);
            pTemp[i+120] = pTemp[i+112] = shift(pFont[2*i+17]);
        }
    }
    else
    {
        for (i = 0; i < 8; i++)
        {
            pTemp[i+8] = pTemp[i] = shift(pFont[i]);
            pTemp[i+40] = pTemp[i+32] = shift(pFont[i]); 
            pTemp[i+24] = pTemp[i+16] = 0;
            pTemp[i+56] = pTemp[i+48] = 0;
            pTemp[i+72] = pTemp[i+64] = shift(pFont[i+8]);
            pTemp[i+104] = pTemp[i+96] = shift(pFont[i+8]);
            pTemp[i+88] = pTemp[i+82] = 0;
            pTemp[i+120] = pTemp[i+112] = 0;
        }
    }
}


/*************************************************
  Function:        LimitSize
  Description:    ?ж????????Χ?????????????С
  Input:        val??????len?????limit???С
  Output:        ??
  Return:        ?????
*************************************************/
static FH_UINT32 LimitSize(FH_UINT32 val, FH_UINT32 len, FH_UINT32 limit)
{
    FH_UINT32 tmp;
    
    if((val + len) > limit)
    {
        // ?????? val ??????????? len ?????
        // ?????????? limit - len, ???? 720 - 64 ?? 64 ??????
        //return (limit - len);
        tmp = val - len;
        if (tmp < 0)
        {
            tmp = 0;
        }
        return tmp;
    }
    else
    {
        return val;
    }
}


/*************************************************
  Function:        InitOsdChar
  Description:    ?????OSD???
  Input:        chan:    ???????
                  2??mode:0:???????,1:???????
                              2:??????
                  line:    ??????
                  code:    ?????????
                  pLeft:    ?????λ??
                  top:    ?????
                  lum:    ??????????
  Output:        NONE
  Return:        ??????
*************************************************/
static int InitOsdChar(FH_UINT32 chan, FH_UINT32 mode,OSD_LINE *pLine,FH_UINT32 code,FH_UINT32 *pLeft,FH_UINT32 top,FH_UINT32 lum,FH_UINT32 needSpace,FH_UINT32 *tag, FH_UINT32 line)
{
    FH_UINT32    charCnt=0;
    FH_UINT32    charType;
    FH_UINT8 *fontAddr=0;
    FH_UINT32    tagIdx=0;
    OSD_CHAR *pChar;
    FH_UINT32    charW=0,charH=0;

    chan=0;
    charType=GET_CHAR_TYPE(code);
    if(charType==OSD_CHAR_TYPE_TAG)
    {
        tagIdx=code-OSD_TAG_CODE_BASE;
        *tag = tagIdx;
        charCnt=osdTagParam[tagIdx].charCnt;
        charW=osdTagParam[tagIdx].charWidth;
        
        charH=16;
        if(osdTagParam[tagIdx].flgEncUsed&(1<<chan))
        {
            *pLeft+=charW*charCnt*(pLine->osdScaleH+1);
            return 0;
        }
        osdTagParam[tagIdx].line = line;
    }
    else
    {
        if(code==' ')
        {
             if(!needSpace)
            {
                *pLeft+=OSD_CHAR_W_ASC*(pLine->osdScaleH+1);
                return 0;
            }
        }
        charCnt=1;
        
        if(charType==OSD_CHAR_TYPE_ASC)
        {
            charW=OSD_CHAR_W_ASC;
            charH=16;
            fontAddr=stAscFontData.pLibData+code*16;            
        }
        else if(charType==OSD_CHAR_TYPE_SPECIAL)
        {
            charH=16;
            charW=OSD_CHAR_W_ASC;
            code-=0x8000;
            
            if(code*16>128*16)
            {
                return  -1;
            }

            fontAddr=stChineseFontData.pLibData+code*16;
            
        }
        else if(charType==OSD_CHAR_TYPE_RS)
        {
            charH=16;
            charW=OSD_CHAR_W_ASC;
            if(code<=0xa7c1)
            {
                code-=0xa7a1;
            }
            else
            {
                code-=((0xa7d1-0xa7c1-1)+0xa7a1);
            }

            if(code*16>FONT_LIB_RS_LEN)
            {
                return -1;
            }

            /* ??ж????????????? */
            //if(!gb2312)
            //{
                //OSD_DBG("InitOsdChar: fontLibis NULL\n");
                //return -1;
            //}

            fontAddr=stChineseFontData.pLibData+code*16;
        }
        else if(charType==OSD_CHAR_TYPE_GREECE)
        {
            charH=16;
            charW=OSD_CHAR_W_ASC;

            if(code<=0xa6b8)
            {
                code-=0xa6a1;
            }
            else
            {
                code-=((0xa6c1-0xa6b8-1)+0xa6a1);
            }
        
            if(code*16>FONT_LIB_GREECE_LEN)
            {
                OSD_DBG("InitOsdChar: INVALID code, type=%x codce=%x", charType, code);
                return -1;
            }
            /* ?????????????????? */
            //if(!gb2312)
            //{
            //    return -1;
            //}

            fontAddr=stChineseFontData.pLibData+code*16;
        }
        else
        {
            charH=16;
            charW=OSD_CHAR_W_HZ;
            if(GET_HZ_OFFSET(code)>FONT_HZ_GBK_LEN)
            {
                OSD_DBG("InitOsdChar: INVALID code, type=%x codce=%x", charType, code);
                OSD_DBG("Font lib overflow,code:%x,offset:%u",code,GET_HZ_OFFSET(code));
                return -1;
            }
            /* ????????????????? */
        //    if(!stChineseFontData.pLibData)
            //{
            //    OSD_DBG("error fontLib \n");
            //    return -1;
            //}
                
            fontAddr=stChineseFontData.pLibData+GET_HZ_OFFSET(code);
            //OSD_DBG("FONT_ADD==0x%x\n",fontAddr);
            OSD_DBG("pchar_code==%d\n",code);
        }
    }
    if((pLine->charCnt+charCnt)>OSD_CHAR_CNT)
    {
        OSD_DBG("InitOsdChar: INVALID charCnt: %x+%x\n", pLine->charCnt, charCnt);
        return -1;
        //RET_ERR(OSD_CHAR_CNT_OVERFLOW);
    }
    pChar = &pLine->charData[pLine->charCnt];
    if (charType == OSD_CHAR_TYPE_TAG)
    {
        osdTagParam[tagIdx].flgEncUsed |= (1 << chan);
        osdTagParam[tagIdx].pFirstChar[chan] = pChar;
        
    }
    while(charCnt--)
    {
        //pChar->top=(top<<16)|((*pLeft));
        pChar->pos_orig=(top<<16)|((*pLeft));
        pChar->size=(charH<<24)+(charW<<16);
        pChar->pFont=fontAddr;
        
        pChar->color=lum;
        pChar->code=code;
        OSD_DBG("PCHAR_CODE==%d\n",pChar->code);
        pChar->charType = charType;
        *pLeft+=charW*(pLine->osdScaleH+1);
        pLine->charCnt++;
        pChar++;
    }
    return 0;
}


/*************************************************
  Function:        RecheckOsdParam
  Description:    ?????????????????????????
                  ???OSD???????????????
  Input:        chan:    ???????
                  mode:    0:???????
                          1:???????
                          2:??????
  Output:        NONE
  Return:        void
*************************************************/
static void RecheckOsdParam(FH_UINT32 chan, FH_UINT32 mode, void *pFrame)
{
    OSD_PARAM *pParam=NULL;
    OSD_LINE  *pLine;
    OSD_CHAR *pChar;
    FH_UINT32 i,j,x,y,w,h;
    //FH_UINT32 imgW,imgH;    
    FH_UINT32 scaleHv;
    
    FH_UINT32 cnt = 0;
    pParam=(OSD_PARAM *)pEncOsdParam.vbase;

   // imgW = pCtrl->encW;
   // imgH = pCtrl->encH;
    
    if(pParam->bReady==FH_FALSE)
    {
        return;
    }
        
    /*************************************/
    /*????????????????????С???????*/
    /*************************************/

    for(i=0;i<OSD_MAX_LINE_BASE;i++)
    {
        pLine=pParam->line+i;
        pLine->validCharCnt=0;
        pLine->occupyCharCnt=0;
        pLine->x=pLine->y=pLine->w=pLine->h=0;
        if ((pLine->osdScaleH == 0) && (pLine->osdScaleV == 0))  
        {
            scaleHv= 1;
        }
        else if ((pLine->osdScaleH == 0) && (pLine->osdScaleV == 1))
        {
            scaleHv= 1;
        }
        else if ((pLine->osdScaleH == 1) && (pLine->osdScaleV == 1))
        {
            scaleHv= 2;
        }
        else
        {
            scaleHv= 1;
        }
        //OSD_DBG("CHARCNT==%d\n",pLine->charCnt);
        for(j=0;j<pLine->charCnt;j++)
        {
            pChar=pLine->charData+j;

            x=(pChar->pos_orig)&0xffff;
            y=pChar->pos_orig>>16;

            w=(pChar->size>>16)&0xff;
            h=pChar->size>>24;
            ////
            
            //x<<= scaleH;

            
            //y????λ???x??w??h?????????仯
            //x*=(scaleH+1);
            //w*=(scaleH+1);
            //h*=(scaleV+1);
            w *= scaleHv;
            h *= scaleHv;
        
            //????????????????????????????????????
            if(j==0)
            {
                pLine->x=x;
                pLine->y=y;
            }
            //???????????????????????????????
            if(j==(pLine->charCnt-1))
            {
                pLine->w=x+w-pLine->x;
            }
            if(h>pLine->h)
            {
                pLine->h=h;
            }
            //?????????OSD?????μ?λ??
            pChar->pos=(y-pLine->y)<<16|(x-pLine->x);
            pChar->size=(pChar->size&0xffff0000)|(h<<8)|w;
            
            cnt += w * h * 2;
            pLine->validCharCnt++;
            if(h > w)
            {
                pLine->occupyCharCnt ++;
            }
            else
            {              
                pLine->occupyCharCnt += 2;
            }
        }    
    }

}

static int set_osd(FH_UINT32 chan, FH_UINT32 mode, FH_UINT32 bStart, OSD_CONFIG *pOsdInit)
{
    FH_UINT32 i, j;
    FH_UINT32 x, y, tmpX, tmpY;
    int err = 0;
    FH_UINT32 lum, tag;
    FH_UINT32 osdScaleH = 0, osdScaleV = 0;
    OSD_PARAM *pParam;
    OSD_CONFIG *pInit = (OSD_CONFIG *)pOsdInit;
    
    chan = 0;
    
    ADVOSD_LOCK;
    pParam = (OSD_PARAM *)pEncOsdParam.vbase;
    flgOsd &= ~(1u<<chan);
    if (pInit->bAutoAdjustLum)
    {
        lum = OSD_WHITE_COLOR;
    }
    else
    {
        lum = pInit->lum;
    }
    pParam->bReady = FH_FALSE;
    pParam->bAutoLum = FH_FALSE;
    pParam->bTranslucent = FH_FALSE;
    pParam->bFlash = FH_FALSE;
    if (pInit->bAutoAdjustLum)
    {
        pParam->bAutoLum = FH_TRUE;
    }
    if (pInit->bTranslucent)
    {
        pParam->bTranslucent = FH_TRUE;
    }
    if (pInit->flash)
    {
        pParam->bFlash = FH_TRUE;
        pParam->flashOn = (pInit->flash >> 8) & 0xff;
        pParam->flashOff = pInit->flash & 0xff;
    }
    for (i = 0; i < OSD_TAG_CNT; i++)
    {
        osdTagParam[i].flgEncUsed &= ~(1u << chan);
    }
    for (i = 0; i < OSD_MAX_LINE_BASE; i++)
    {

        if (pInit->line[i].charCnt > MAX_CHAR_OSD_LINE
            || pInit->line[i].charCnt == 0)
        {
            pParam->line[i].charCnt = 0;
            pParam->line[i].validCharCnt = 0;
            continue;
        }
        x = pInit->line[i].x;
        y = pInit->line[i].top;
        pParam->line[i].charCnt = 0;
        pParam->line[i].validCharCnt = 0;
        pParam->line[i].LINE_COLOR = lum;
        
        tmpX = x;
        tmpY = y;    
        switch (pInit->osdSize[chan])
        {
            case OSD_SIZE_16_16:
                if((tmpX > 0) && (tmpX < 8))
                {
                    tmpX = 8;
                }
                if((tmpY > 0) && (tmpY < 8))
                {
                    tmpY = 8;
                }                    
                x = ((tmpX + 8) / 16) * 16;
                y = ((tmpY + 8) / 16) * 16;            
                x = LimitSize(x, 16, OsdInitW);
                y = LimitSize(y, 16, OsdInitH);
                osdScaleH = 0;
                osdScaleV = 0;                    
                break;
            case OSD_SIZE_32_32:
                if ((tmpX > 0) && (tmpX < 16))
                {
                    tmpX = 16;
                }
                if ((tmpY > 0) && (tmpY < 16))
                {
                    tmpY = 16;
                }                    
                x = ((tmpX + 16) / 32) * 32;
                y = ((tmpY + 16) / 32) * 32;            
                x = LimitSize(x, 32, OsdInitW);
                y = LimitSize(y, 32, OsdInitH);                
                osdScaleH = 0;
                osdScaleV = 1;                
                break;
            
            case OSD_SIZE_64_64:
                if ((tmpX > 0) && (tmpX < 32))
                {
                    tmpX = 32;
                }
                if ((tmpY > 0) && (tmpY < 32))
                {
                    tmpY = 32;
                }                    
                x = ((tmpX + 32) / 64) * 64;
                y = ((tmpY + 32) / 64) * 64;
                x = LimitSize(x, 64, OsdInitW);
                y = LimitSize(y, 64, OsdInitH);        
                osdScaleH = 1;
                osdScaleV = 1;            
                break;                
            case OSD_SIZE_AUTO:
                if (((OsdInitW * OsdInitH) >= 1280*720) && (OsdInitW >= 720))
                {
                    if ((tmpX > 0) && (tmpX < 32))
                    {
                        tmpX = 32;
                    }
                    if ((tmpY > 0) && (tmpY < 32))
                    {
                        tmpY = 32;
                    }                    
                    x = ((tmpX + 32) / 64) * 64;
                    y = ((tmpY + 32) / 64) * 64;        
                    x = LimitSize(x, 64, OsdInitW);
                    y = LimitSize(y, 64, OsdInitH);                    
                    osdScaleH = 1;
                    osdScaleV = 1;        
                }
                else if(((OsdInitW * OsdInitH) >= 640*480) && (OsdInitW >= 480))
                {
                    if ((tmpX > 0) && (tmpX < 16))
                    {
                        tmpX = 16;
                    }
                    if ((tmpY > 0) && (tmpY < 16))
                    {
                        tmpY = 16;
                    }                    
                    x = ((tmpX + 16) / 32) * 32;
                    y = ((tmpY + 16) / 32) * 32;            
                    x = LimitSize(x, 32, OsdInitW);
                    y = LimitSize(y, 32, OsdInitH);                    
                    osdScaleH = 0;
                    osdScaleV = 1;        
                }
                else
                {    
                    if ((tmpX > 0) && (tmpX < 8))
                    {
                        tmpX = 8;
                    }
                    if ((tmpY > 0) && (tmpY < 8))
                    {
                        tmpY = 8;
                    }                    
                    x = ((tmpX + 8) / 16) * 16;
                    y = ((tmpY + 8) / 16) * 16;            
                    x = LimitSize(x, 16, OsdInitW);
                    y = LimitSize(y, 16, OsdInitH);                    
                    osdScaleH = 0;
                    osdScaleV = 0;        
                }
                break;
        }
        if((osdScaleH == 0 && osdScaleV == 0)||(osdScaleH == 0 && osdScaleV == 1))
        {
            pParam->line[i].pDrawFunc = DrawCharH1V1;
        }
        else if(osdScaleH == 1 && osdScaleV == 1)
        {
            pParam->line[i].pDrawFunc = DrawCharH2V2;
        }
        else
        {
            osdScaleH = osdScaleV = 1;
            pParam->line[i].pDrawFunc = DrawCharH2V2;
        }
            
        pParam->line[i].osdScaleH = osdScaleH;
        pParam->line[i].osdScaleV = osdScaleV;
        for (j = 0; j < pInit->line[i].charCnt; j++)
        {
            err = InitOsdChar(chan, mode, pParam->line+i, pInit->line[i].code[j], &x, y, lum, 1, &tag, i);
            if (err)
            {
                OSD_DBG("ERR in init osd char\n");
                ADVOSD_UNLOCK;
                return err;
            }
        }
    }
    
    pParam->bReady=FH_TRUE;    
    if (bStart)
    {
        RecheckOsdParam(0, mode, NULL);
        flgOsd |= 1u << 0;
        _updateNowtime(); // 2015.10 new add
        RefreshOsdTag(0xffffffff, mode, (DATE_TIME *)(&now), 0);
        dtOld = now; // 2015.10 new add
        draw_osd(chan, mode);
    }
    
    ADVOSD_UNLOCK;
    return err;
}

// pGraph?NULL ????????gosdλ?????? 2015.08.27 
static int set_logo( FH_AREA  rect, unsigned int alpha,unsigned char *pGraph)
{
    int errs;
    FH_UINT32     m;
    FH_VPU_LOGO vpu_logo_info;
    unsigned char *pData;

    if(NULL != pGraph)
    {
        pData = pGraph;    
        for(m = 0;m < rect.u32Width*rect.u32Height;m++)
        {
            *(unsigned char *)(pLogoBuf.vbase  + 2*m) = pData[2*m];
            *(unsigned char *)(pLogoBuf.vbase  + 2*m + 1)= pData[2*m+1];
        }
    }
    
    vpu_logo_info.logo_cfg.alpha_value = alpha;
    vpu_logo_info.logo_addr = (unsigned char *)pLogoBuf.base;
    vpu_logo_info.logo_cfg.logo_size.u32Width= rect.u32Width;
    vpu_logo_info.logo_cfg.logo_size.u32Height= rect.u32Height;
    vpu_logo_info.logo_cfg.dtvmode = 0;
    vpu_logo_info.logo_cfg.rgbmode = 0;
    vpu_logo_info.logo_startpos.u32X= rect.u32X;
    vpu_logo_info.logo_startpos.u32Y= rect.u32Y;
    vpu_logo_info.stride_value = 2*rect.u32Width;
	vpu_logo_info.logo_cutpos.u32X = 0;
	vpu_logo_info.logo_cutpos.u32Y = 0;
    vpu_logo_info.logo_enable = 1;

    
    errs = FH_VPSS_SetGraph(&vpu_logo_info);
    if(errs != 0)
    {
        OSD_DBG("FH_VPSS_SetGraph_errs == %d\n",errs);
    }
    return errs;
}

typedef struct {
    int thread_running;
    rt_thread_t thread_id;
} osd_thread_t;

static osd_thread_t *OsdThreadHandle = RT_NULL;

static void _OSD_taskbody(void *param)
{
    while (OsdThreadHandle->thread_running)
    {
        set_invert();
        if(osdTimeFlush)
            RefreshDateTime();
        SystemSleep(200);
    }
}


///////////////////////////////////////////////////////
static unsigned int timeOsdDataFormat[12][8]={
            {OSD_YEAR4, 0x2d, OSD_MONTH2, 0x2d, OSD_DAY, 0x20, 0x20}, //Y-M-D    //??????
            {OSD_MONTH2, 0x2d, OSD_DAY, 0x2d, OSD_YEAR4, 0x20, 0x20}, //M-D-Y
            {OSD_DAY, 0x2d, OSD_MONTH2, 0x2d, OSD_YEAR4, 0x20, 0x20}, //D-M-Y
            {OSD_YEAR4, 0x2f, OSD_MONTH2, 0x2f, OSD_DAY, 0x20, 0x20}, //Y/M/D
            {OSD_MONTH2, 0x2f, OSD_DAY, 0x2f, OSD_YEAR4, 0x20, 0x20}, //M/D/Y
            {OSD_DAY, 0x2f, OSD_MONTH2, 0x2f, OSD_YEAR4, 0x20, 0x20}, //D/M/Y
            {OSD_YEAR4, 0x2d, OSD_MONTH2, 0x2d, OSD_DAY, 0x20, 0x20}, //Y-M-D   /////// ???????
            {OSD_MONTH2, 0x2d, OSD_DAY, 0x2d, OSD_YEAR4, 0x20, 0x20}, //M-D-Y
            {OSD_DAY, 0x2d, OSD_MONTH2, 0x2d, OSD_YEAR4, 0x20, 0x20}, //D-M-Y
            {OSD_YEAR4, 0xc4ea, OSD_MONTH2, 0xd4c2, OSD_DAY, 0xc8d5, 0x20, 0x20}, //??????
            {OSD_MONTH2, 0xd4c2, OSD_DAY, 0xc8d5, OSD_YEAR4, 0xc4ea, 0x20, 0x20}, //??????
            {OSD_DAY, 0xc8d5, OSD_MONTH2, 0xd4c2, OSD_YEAR4, 0xc4ea, 0x20, 0x20}, //??????
};

static unsigned int timeosdWeek[2][2]= {
            {OSD_WEEK3, 0x20},  //???????
            {OSD_CWEEK1, 0x20},   //????????

};
static unsigned int timeosdTime[2][9] = {
            {OSD_AMPM, 0x20, OSD_HOUR12, 0x3a, OSD_MINUTE, 0x3a, OSD_SECOND,0x20,0x20}, //12С???
            {OSD_HOUR24, 0x3a, OSD_MINUTE, 0x3a, OSD_SECOND,0x20,0x20},   //24С???
};
static int _SetOsdFormat(OSD_TimeFmt_e type, unsigned char week, unsigned char nrom, unsigned int *pCode)
{
    int length = 0;

    memcpy(pCode + length, timeOsdDataFormat[type], sizeof(timeOsdDataFormat[type]));

    if (type <FHTEN_OSD_TimeFmt9)
        length = sizeof(timeOsdDataFormat[type])/4 - 2;
    else
        length = sizeof(timeOsdDataFormat[type])/4;
    
    if(week)
    {
        if(type < FHTEN_OSD_TimeFmt6)
        {
            memcpy(pCode + length,timeosdWeek[0],sizeof(timeosdWeek[0]));
            length += (sizeof(timeosdWeek[0])/4);
        }
        else
        {

            memcpy(pCode + length,timeosdWeek[1],sizeof(timeosdWeek[1]));
            length += (sizeof(timeosdWeek[1])/4);
        }
            

    }
#if 0    
   if(nrom)
       memcpy(pCode + length,timeosdTime[0],sizeof(timeosdTime[0]));
   else
       memcpy(pCode + length,timeosdTime[1],sizeof(timeosdTime[1]));
#else
    memcpy(pCode + length,timeosdTime[1],sizeof(timeosdTime[1]));
#endif   

    if(type > FHTEN_OSD_TimeFmt5 && week)
        length += 2; //????????
    return length+(sizeof(timeosdTime[1])/4);
}


static unsigned int _osd_rgb2yuv(unsigned int alpha, unsigned int r, unsigned int g, unsigned int b)
{
    unsigned int y;
    unsigned int cb;
    unsigned int cr;

    unsigned int color = 0;

    y = (0.299*r + 0.587*g + 0.114*b + 0);
    cb = (- 0.169*r - 0.331*g + 0.500*b + 128);
    cr = (0.500*r - 0.419*g - 0.081*b + 128);

    color = (alpha <<24) |(y <<16)| (cb << 8) | (cr);
    return color;
}
/////////////////////////////////////////

FH_SINT32 osd_init(int width, int hight)
{
    int ret = 0;
    static int _osd_init = 0;

    if (_osd_init)
        return 0;
    ret = init_osd(width,hight);
    if (ret != 0)
        return ret;

    memset(&stChineseFontData,0,sizeof(stChineseFontData));
    memset(&stAscFontData,0,sizeof(stAscFontData));

    sOsdLockhandle = rt_mutex_create("osd_lock", RT_IPC_FLAG_FIFO);
    if (sOsdLockhandle == RT_NULL)
        return -1;
    _osd_init = 1;

    return 0;
}

FH_SINT32 osd_load_fontlib(OSD_FontType_e type, OSD_FontLib_t *pFontData)
{
    if (pFontData == RT_NULL)
        return -1;

    if(type == FHEN_FONT_TYPE_ASC)
    {
        stAscFontData.fontSize = pFontData->fontSize;
        stAscFontData.pLibData = pFontData->pLibData; // ?????????????
    }
    else if(type == FHEN_FONT_TYPE_CHINESE)
    {
        stChineseFontData.fontSize = pFontData->fontSize;
        stChineseFontData.pLibData = pFontData->pLibData; // ?????????????
    }

    return 0;
}

FH_SINT32 osd_unload_fontlib(OSD_FontType_e type)
{
    OSD_FontLib_t *pstFontLib;
    
    if (type == FHEN_FONT_TYPE_ASC)
        pstFontLib = &stAscFontData;
    else
        pstFontLib = &stChineseFontData;

    if (pstFontLib->pLibData != RT_NULL)
    {
        pstFontLib->pLibData = RT_NULL; 
        pstFontLib->fontSize = 0;
    }

    return 0;
}

static OSD_CONFIG Time_Form;

FH_SINT32 osd_set_text(OSD_CONFIG_t *pOsdcfg)
{
    int ret = 0;
    int i =0;
    int j = 0;
    int u32Index = 0;
    int codeIndex = 0;
    int flagIndex = 0;
    int charCount[8] = {0};

    if (RT_NULL == stChineseFontData.pLibData || RT_NULL ==stAscFontData.pLibData)
    {
        OSD_DBG("[fhadv_osd]the ASC or Chinese font lib is NULL\n");
        return -1;
    }
    if(pOsdcfg->timeOsdEnable)
    {
        if (pOsdcfg->timeOsdPosition.pos_y <0 || pOsdcfg->timeOsdPosition.pos_x < 0 \
            ||pOsdcfg->timeOsdPosition.pos_x >OsdInitW ||pOsdcfg->timeOsdPosition.pos_y >OsdInitH)
        {
            OSD_DBG("[fhadv_osd]time osd position is error\n");
            return -1;
        }
        osdTimeFlush = 1;
    }
    else
        osdTimeFlush = 0;

    if (pOsdcfg->text01Enable)
    {
        if(pOsdcfg->sttext01Position.pos_x < 0 || pOsdcfg->sttext01Position.pos_y < 0 \
            ||pOsdcfg->sttext01Position.pos_x >OsdInitW ||pOsdcfg->sttext01Position.pos_y >OsdInitH)
        {
            OSD_DBG("[fhadv_osd]osd text1 position is error\n");
            return -1;
        }
    }

    if (pOsdcfg->text02Enable)
    {
        if(pOsdcfg->sttext02Position.pos_x < 0 || pOsdcfg->sttext02Position.pos_y < 0 \
            ||pOsdcfg->sttext02Position.pos_x >OsdInitW ||pOsdcfg->sttext02Position.pos_y >OsdInitH)
        {
            OSD_DBG("[fhadv_osd]osd text2 position is error\n");
            return -1;
        }
    }
    
    memset(&Time_Form,0, sizeof(OSD_CONFIG));
    Time_Form.flash = 0;
    Time_Form.bAutoAdjustLum = 0;
    Time_Form.lum = _osd_rgb2yuv(255, pOsdcfg->osdColor.norcolor.fRed, pOsdcfg->osdColor.norcolor.fGreen, pOsdcfg->osdColor.norcolor.fBlue);
    Time_Form.bTranslucent = FH_TRUE;
    Time_Form.line[0].top = pOsdcfg->timeOsdPosition.pos_y;
    Time_Form.line[0].x = pOsdcfg->timeOsdPosition.pos_x;
    Time_Form.line[0].y = pOsdcfg->timeOsdPosition.pos_y;
    Time_Form.osdSize[0] = 1;
    Time_Form.osdSize[1] = 1;
    Time_Form.osdSize[2] = 1;
    if (pOsdcfg->timeOsdEnable)
        Time_Form.line[0].charCnt =_SetOsdFormat(pOsdcfg->timeOsdFormat, pOsdcfg->weekFlag, pOsdcfg->timeOsdNorm, Time_Form.line[0].code);
    else
        Time_Form.line[0].charCnt = 0;


    while('\0' != pOsdcfg->text01Info[u32Index])
    {
        if(u32Index >31)
            break;
        if(pOsdcfg->text01Info[u32Index] &0x80)
        {
            if(flagIndex > 30)
                break;
            Time_Form.line[1].code[codeIndex] = (pOsdcfg->text01Info[u32Index]<<8) | pOsdcfg->text01Info[u32Index+1];
            if('\0' == pOsdcfg->text01Info[u32Index+1])
                break;
            u32Index += 2;
            flagIndex += 2;
        }
        else
        {
            if(flagIndex >31)
                break;
            Time_Form.line[1].code[codeIndex] = pOsdcfg->text01Info[u32Index];
            u32Index ++;
            flagIndex++;
        }
        codeIndex ++;
    }

    if (pOsdcfg->text01Enable)
        Time_Form.line[1].charCnt = flagIndex;
    else
        Time_Form.line[1].charCnt = 0;

    Time_Form.line[1].top = pOsdcfg->sttext01Position.pos_y;
    Time_Form.line[1].x = pOsdcfg->sttext01Position.pos_x;
    Time_Form.line[1].y = pOsdcfg->sttext01Position.pos_y;
    u32Index = 0;
    codeIndex = 0;
    flagIndex = 0;



    i = 1;
    while('\0' != pOsdcfg->text02Info[u32Index])
    {
        if(u32Index >239)
            break;
        if(pOsdcfg->text02Info[u32Index] & 0x80)
        {
            if((flagIndex +2)>32)
            {
                Time_Form.line[1+i].code[codeIndex] = 0x20;
                charCount[i-1] = 32;
                codeIndex = 0;
                flagIndex = 0;
                i++;
                if (9 == i)
                    break;                
            }
            Time_Form.line[1+i].code[codeIndex] = (pOsdcfg->text02Info[u32Index]<<8) | pOsdcfg->text02Info[u32Index+1];
            if('\0' == pOsdcfg->text02Info[u32Index+1])
                break;
     
            u32Index+=2;
            flagIndex+=2;
            charCount[i-1] = flagIndex;
        }
        else
        {
            if((flagIndex +1)>32)
            {
                charCount[i-1] = 32;
                codeIndex = 0;
                flagIndex = 0;
                i++;

                if (9 == i)
                    break;
            }
            Time_Form.line[1+i].code[codeIndex] = pOsdcfg->text02Info[u32Index];
            u32Index ++;
            flagIndex++;
            charCount[i-1] = flagIndex;
        }
        codeIndex++;
        
    }
    for(j = 0; j < 8; j++)
    {
        if(pOsdcfg->text02Enable)
            Time_Form.line[j+2].charCnt = charCount[j];
        else
            Time_Form.line[j+2].charCnt = 0;

        Time_Form.line[j+2].top = pOsdcfg->sttext02Position.pos_y;
        Time_Form.line[j+2].x = pOsdcfg->sttext02Position.pos_x;
        Time_Form.line[j+2].y = pOsdcfg->sttext02Position.pos_y;
    }

    stOsdColor.norcolor = _osd_rgb2yuv(255, pOsdcfg->osdColor.norcolor.fRed, pOsdcfg->osdColor.norcolor.fGreen, pOsdcfg->osdColor.norcolor.fBlue);
    stOsdColor.invcolor= _osd_rgb2yuv(255, pOsdcfg->osdColor.invcolor.fRed, pOsdcfg->osdColor.invcolor.fGreen, pOsdcfg->osdColor.invcolor.fBlue);

    stOsdbgColor.norcolor = 0;
    stOsdbgColor.invcolor= 0;

    ret = set_osd(0, 0, 1, &Time_Form);

    if(OsdThreadHandle == NULL)
    {
        OsdThreadHandle = rt_malloc(sizeof(osd_thread_t));
        if (OsdThreadHandle == RT_NULL)
            return -1;
        OsdThreadHandle->thread_running = 0;
        OsdThreadHandle->thread_id = rt_thread_create("OSDThread",
                                                      _OSD_taskbody,
                                                      RT_NULL,
                                                      1024,
                                                      RT_THREAD_PRIORITY_MAX / 2 + 11,
                                                      2);

        rt_thread_startup(OsdThreadHandle->thread_id);
        OsdThreadHandle->thread_running = 1;
    }

    return ret;
}

FH_SINT32 osd_set_text_rotate(FH_SINT32 enable, FH_SINT32 value)
{
    rotate_flag  = enable;
    rotate_value = value;
    ADVOSD_LOCK;
    draw_osd(0,0);
    ADVOSD_UNLOCK;
    return 0;
}

FH_SINT32 osd_set_logo(OSD_Logo_t *pLogoCfg)
{
    int ret = 0;
    FH_AREA   rect;
    FH_VPU_LOGO vpu_logo_info;
    vpu_logo_info.logo_enable =0;
    rect.u32X = pLogoCfg->area.fTopLeftX;
    rect.u32Y = pLogoCfg->area.fTopLeftY;
    rect.u32Width = pLogoCfg->area.fWidth;
    rect.u32Height = pLogoCfg->area.fHeigh;
    if(pLogoCfg->enable)
        ret = set_logo(rect,pLogoCfg->alpha,pLogoCfg->pData);
    else  // disable 2018.08.27 change
    {
         if(NULL != pLogoCfg->pData) // ????????? gosd data ?????
         {
             int _i;
             unsigned char *_p = pLogoCfg->pData;    
             for(_i = 0;_i < (pLogoCfg->area.fWidth) * (pLogoCfg->area.fHeigh);_i++)
             {
                 *(unsigned char *)(pLogoBuf.vbase  + 2*_i) = _p[2*_i];
                 *(unsigned char *)(pLogoBuf.vbase  + 2*_i + 1)= _p[2*_i+1];
             }
         }
        ret = FH_VPSS_SetGraph(&vpu_logo_info);
    }    
    return ret;
}

void osd_uninit(void)
{
    rt_thread_delete(OsdThreadHandle->thread_id);
    rt_free(OsdThreadHandle);
    OsdThreadHandle = NULL;
}