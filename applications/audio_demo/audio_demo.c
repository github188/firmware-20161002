#include <rtdef.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <dfs_posix.h>

#include "acw.h"

#define BUFF_SIZE  1024*8
static rt_uint32_t rx_buff[BUFF_SIZE] __attribute__((aligned(32)));
static rt_uint32_t tx_buff[BUFF_SIZE] __attribute__((aligned(32)));

static unsigned char txTmp[4096];
static unsigned char txExtTmp[8192];

#define STRINGFILE	"/32.dat"
#define STRINGFILE2	"/16.wav"

static void audio_demo_main(void* parameter)
{
    rt_kprintf("audio_demo_cap(\"music.wav\") capture 1M data\n");
    rt_kprintf("audio_demo_play(\"music.wav\") play 1M data\n");
}

/******************************
 function:
 1.capture pcm data
 2.write file
 3.convert 32bit to 16bit
 4.add wave header
 *****************************/

//////default waveHeader//////////////////
unsigned char waveHeader[] =
{ 0x52, 0x49, 0x46, 0x46, 0x30, 0x00, 0x08, 0x00, 0x57, 0x41, 0x56, 0x45, 0x66,
        0x6d, 0x74, 0x20, 0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x40,
        0x1f, 0x00, 0x00, 0x80, 0x3e, 0x00, 0x00, 0x02, 0x00, 0x10, 0x00, 0x66,
        0x61, 0x63, 0x74, 0x04, 0x00, 0x00, 0x00, 0x00, 0xbe, 0x00, 0x00, 0x64,
        0x61, 0x74, 0x61 };

static void audio_demo_cap(void* parameter)
{
    struct fh_audio_cfg_arg cfg;
    rt_device_t acw_dev;

    int i;
    int output = 3;
    int select;
    int fd;
    int fd2;
    int length;

    rt_uint32_t tick_start, tick_end;

    char buffer[4];
    int read_len;
    unsigned short data;
    unsigned int *ptr;
    int rx_read_szie = 0;
    int count = 0;
    unsigned char tmp = 0;

    char waveFile[64];
    strcpy(waveFile, (char*) parameter);

    acw_dev = (rt_device_t) rt_device_find("audio");

    for (i = 0; i < BUFF_SIZE; i++)
        rx_buff[i] = i * 0x500;

    acw_dev->open(acw_dev, 0);

    rt_kprintf("line %d addr %x \n", __LINE__, &rx_buff[0]);

    cfg.buffer_size = BUFF_SIZE;
    cfg.period_size = BUFF_SIZE / 8;

    rt_kprintf("line %d  \n", __LINE__);

    cfg.channels = 0;
    cfg.frame_bit = 16;
    cfg.io_type = mic_in;

    cfg.rate = 8000;

    tmp = cfg.rate % 0xff;
    waveHeader[24] = tmp;
    tmp = cfg.rate / 0xff;
    waveHeader[25] = tmp;

    tmp = cfg.rate * 2 % 0xff;
    waveHeader[28] = tmp;
    tmp = cfg.rate * 2 / 0xff;
    waveHeader[29] = tmp;

    cfg.volume = 70;

    acw_dev->control(acw_dev, AC_INIT_CAPTURE_MEM, &cfg);
    acw_dev->control(acw_dev, AC_AI_EN, &cfg);

    fd = open(STRINGFILE, O_RDWR | O_CREAT | O_TRUNC, 0);
    if (fd < 0)
    {
        rt_kprintf("open file for failed\n");

        return;
    }

    while (1)
    {
        rx: select = acw_dev->rx_indicate(acw_dev, RT_NULL);
        if (!select)
            goto rx;

        rx_read_szie = acw_dev->read(acw_dev, 0, &rx_buff[0], 1024 * 8);

        length = write(fd, &rx_buff[0], 1024 * 8);

        if (length != 1024 * 8)
        {
            rt_kprintf("audio write data failed\n");

            close(fd);

            return;
        }

        count++;

        if (count > 128)
        {
            acw_dev->control(acw_dev, AC_AI_DISABLE, &cfg);

            break;
        }
    }

    rt_kprintf("audio capture 1M over\n");

    fd2 = open(waveFile, O_RDWR | O_CREAT | O_TRUNC, 0);
    if (fd < 0)
    {
        rt_kprintf("open file2 for failed\n");

        return;
    }

    lseek(fd, 0, SEEK_SET);

    count = 0;

    write(fd2, waveHeader, 56);

    while (1)
    {
        read_len = read(fd, buffer, 4);

        if (read_len < sizeof(unsigned short))
        {
            break;
        }

        data = ((unsigned int *) buffer)[0] >> 8;

        write(fd2, &data, 2);

        count++;

        if (count >= 1024 * 256)
            break;
    }

    rt_kprintf("convert finish\n");

    close(fd);
    close(fd2);
}

static void audio_demo_play(void* parameter)
{
    struct fh_audio_cfg_arg cfg;
    rt_device_t acw_dev;
    int i;
    int output = 3;
    int select;
    int fd;
    int fd2;
    int length;
    int ret = 0;

    char buffer[4];
    int count = 0;
    int readPos = 0;
    char waveFile[64];

    strcpy(waveFile, (char*) parameter);

    acw_dev = (rt_device_t) rt_device_find("audio");

    acw_dev->open(acw_dev, 0);

    rt_kprintf("after acw_dev open \n");

    cfg.buffer_size = BUFF_SIZE;
    cfg.period_size = BUFF_SIZE / 8;

    cfg.channels = 0;
    cfg.frame_bit = 16;
    cfg.io_type = line_out;

    cfg.rate = 8000;
    cfg.volume = 70;

    cfg.io_type = line_out;
    acw_dev->control(acw_dev, AC_INIT_PLAYBACK_MEM, &cfg);
    acw_dev->control(acw_dev, AC_AO_EN, &cfg);
    acw_dev->control(acw_dev, AC_SET_OUTPUT_MODE, &output);

    fd = open(waveFile, O_RDWR, 0);
    if (fd < 0)
    {
        rt_kprintf("open file for failed\n");

        return;
    }

    lseek(fd, 0, SEEK_SET);

    read(fd, waveHeader, 56);

    while (1)
    {
        length = read(fd, &txTmp[0], 1024 * 4);

        if (length != 1024 * 4)
        {
            rt_kprintf("file read data failed le length = %d\n", length);

            close(fd);
            return;
        }

        readPos += 1024 * 4;

        for (i = 0; i < 1024 * 2; i++)
        {
            txExtTmp[4 * i] = 0x00;
            txExtTmp[4 * i + 1] = txTmp[2 * i];
            txExtTmp[4 * i + 2] = txTmp[2 * i + 1];
            txExtTmp[4 * i + 3] = 0x00;
        }

        memcpy(&tx_buff[0], txExtTmp, 1024 * 8);

        tx: select = acw_dev->tx_complete(acw_dev, RT_NULL);

        if (!select)
            goto tx;

        acw_dev->write(acw_dev, 0, &tx_buff[0], 1024 * 8);

        if (readPos >= (1024 * 500))
        {
            rt_kprintf("play over!!!!!!!\n");

            acw_dev->control(acw_dev, AC_AO_DISABLE, &cfg);

            break;
        }
    }
    close(fd);
}

void audio_demo_init()
{
    rt_thread_t threadAudio;

    threadAudio = rt_thread_create("audio", audio_demo_main, RT_NULL, 10 * 1024,
            8, 20);

    if (threadAudio != RT_NULL)
        rt_thread_startup(threadAudio);
}

#ifdef RT_USING_FINSH
#include <finsh.h>
FINSH_FUNCTION_EXPORT(audio_demo_main, audio_demo_main(audioChn , audioSample , audioBitwidth));
FINSH_FUNCTION_EXPORT(audio_demo_cap,  audio_demo_cap(audioChn , audioSample , audioBitwidth));
FINSH_FUNCTION_EXPORT(audio_demo_play, audio_demo_play(audioChn , audioSample , audioBitwidth));
#endif
