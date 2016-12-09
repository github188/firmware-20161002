#include <finsh.h>

#define FH_DGB_ISP_PROC
#define FH_DGB_DSP_PROC

#ifdef  FH_DGB_ISP_PROC
extern int isp_read_proc();
FINSH_FUNCTION_EXPORT(isp_read_proc,read proc info);
#endif

#ifdef  FH_DGB_DSP_PROC
extern int media_read_proc();
extern int media_mem_proc();
extern int vpu_read_proc();
extern int vpu_write_proc(char *s);
extern int enc_read_proc();
extern int enc_write_proc(char *s);
extern int jpeg_read_proc();
extern int jpeg_write_proc(char *s);
extern int vou_read_proc();
extern int vou_write_proc(char *s);


FINSH_FUNCTION_EXPORT(media_mem_proc, media mem use info. e.g: media_mem_proc());
FINSH_FUNCTION_EXPORT(jpeg_write_proc,write jpeg proc info);
FINSH_FUNCTION_EXPORT(jpeg_read_proc,read jpeg proc info);
FINSH_FUNCTION_EXPORT(media_read_proc,get media process proc info);
FINSH_FUNCTION_EXPORT(enc_write_proc,write enc proc info);
FINSH_FUNCTION_EXPORT(enc_read_proc,read enc proc info);
FINSH_FUNCTION_EXPORT(vou_write_proc,write vou proc info);
FINSH_FUNCTION_EXPORT(vou_read_proc,read vou proc info);
FINSH_FUNCTION_EXPORT(vpu_write_proc,write vpu proc info);
FINSH_FUNCTION_EXPORT(vpu_read_proc,read vpu proc info);
#endif
