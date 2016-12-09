#ifndef _OSD_H_
#define _OSD_H_

#define OSD_LINEONE_NUM 32
#define OSD_LINETWO_NUM 240

typedef unsigned int     UINT;
typedef unsigned char    UCHAR;

typedef struct
{
    int pos_x;
    int pos_y;
}LeftTopNode_t;

/*!矩形结构体*/
typedef struct
{
    int fTopLeftX;
    int fTopLeftY;
    int fWidth;
    int fHeigh;
}Rect_t;

// 时间结构体
typedef struct 
{
    int  year;      /*!< year 1970 ~2039 现加了1900*/
    int  month;     /*!< month (1-12) 本为0起 现加了1*/
    int  day;       /*!< day of the month(1-31)*/
    int  wday;      /*!< day of week (0-6)*/
    int  hour;      /*!< hours (0 - 23)    */
    int  minute;    /*!< minutes (0 - 59)*/
    int  second;    /*!< seconds (0 - 59)*/
}BranchTime_t;

typedef struct
{
    UCHAR fRed;
    UCHAR fGreen;
    UCHAR fBlue;
}RgbColor_t;

typedef union
{
    BranchTime_t   branchTime;
    unsigned long long usTimeStamp;
}SysTime_u;

typedef struct
{
    SysTime_u  timediscribe;
    unsigned char  timeMode; // 0: 分项标示  1: 时间戳模式 us
}SysTime_t;

//osd时间格式
typedef enum
{
    FHTEN_OSD_TimeFmt0 = 0,//Y-M-D 00:00:00
    FHTEN_OSD_TimeFmt1,    //M-D-Y 00:00:00  
    FHTEN_OSD_TimeFmt2,    //D-M-Y 00:00:00
    FHTEN_OSD_TimeFmt3,    //年月日 00:00:00  or Y/M/D 00:00:00  中英文自适应
    FHTEN_OSD_TimeFmt4,    //月日年 00:00:00  or M/D/Y 00:00:00  中英文自适应
    FHTEN_OSD_TimeFmt5,    //日月年 00:00:00  or D/M/Y 00:00:00  中英文自适应
    FHTEN_OSD_TimeFmt6,    // reserve
    FHTEN_OSD_TimeFmt7,    // reserve
    FHTEN_OSD_TimeFmt8,    // reserve
    FHTEN_OSD_TimeFmt9,    // reserve
    FHTEN_OSD_TimeFmt10,    // reserve
    FHTEN_OSD_TimeFmt11,    // reserve
    FHTEN_OSD_TimeFmt_BUTT,//判断限值 非交互项  上层不必关心 

    FHTEN_OSD_END = 0xffffffff
}OSD_TimeFmt_e;

//字库类型枚举
typedef enum
{
    FHEN_FONT_TYPE_ASC,           /*英文字符字库*/
    FHEN_FONT_TYPE_CHINESE,          /*中文字符字库*/

    FHTEN_FONT_TYPE_END = 0xffffffff
}OSD_FontType_e;

typedef struct
{
    UINT       fontSize;
    UCHAR       *pLibData;
}OSD_FontLib_t;

typedef struct
{ 
    RgbColor_t    norcolor;
    RgbColor_t    invcolor;
}OSD_Color_t;

typedef struct
{
    UINT                       timeOsdEnable;      //时间OSD使能项
    LeftTopNode_t          timeOsdPosition;  //时间OSD坐标

    OSD_TimeFmt_e          timeOsdFormat;      //时间OSD格式 
    UCHAR                      timeOsdNorm;        //0: 24小时制 1:12小时制   (暂不支持12小时制)
    UCHAR                      weekFlag;           //星期显示标志 与中英文自动匹配 new add ...

    OSD_Color_t            osdColor;             //OSD颜色

    UINT                        text01Enable;   //自定义文本1使能
    LeftTopNode_t          sttext01Position;  //自定义文本1坐标
    UCHAR          text01Info[OSD_LINEONE_NUM];  //自定义文本1内容

    UINT                        text02Enable;   //自定义文本2使能
    LeftTopNode_t          sttext02Position;  //自定义文本2坐标
    UCHAR          text02Info[OSD_LINETWO_NUM];  //自定义文本2内容
    
}OSD_CONFIG_t;

typedef struct
{
    UINT enable;
    UINT alpha;
    Rect_t      area;
    UCHAR *pData;
}OSD_Logo_t;

int osd_init(int width, int hight);
void osd_uninit(void);
int osd_load_fontlib(OSD_FontType_e type, OSD_FontLib_t *pFontData);
int osd_unload_fontlib(OSD_FontType_e type);
int osd_set_text(OSD_CONFIG_t *pOsdcfg);
int osd_set_text_rotate(int enable, int value);
int osd_set_logo(OSD_Logo_t *pLogoCfg);

#endif
