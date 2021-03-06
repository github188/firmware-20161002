#include <rtthread.h>	//include rtthread.h to use setting in rtconfig.h
#include "multi_sensor.h"

#ifdef RT_USING_GC1024

#define _NAME(n,s)	n##_##s
#ifdef FH_USING_MULTI_SENSOR
#define NAME(n)		_NAME(n,gc1024)
#else
#define NAME(n)		n
#endif

char NAME(isp_param_buff)[FH_ISP_PARA_SIZE] = {
	0x25, 0x12, 0x15, 0x20, //0x0000
	0x00, 0x00, 0x00, 0x00, //0x0004
	0x00, 0x00, 0x00, 0x00, //0x0008
	0x00, 0x02, 0x00, 0x00, //0x000c
	0x07, 0xcd, 0x7d, 0x01, //0x0010
	0x00, 0x05, 0xd0, 0x02, //0x0014
	0x00, 0x0f, 0xee, 0x02, //0x0018
	0x00, 0x00, 0x00, 0x00, //0x001c
	0x00, 0x05, 0xd0, 0x02, //0x0020
	0x00, 0x05, 0xd0, 0x02, //0x0024
	0x00, 0x0f, 0xee, 0x02, //0x0028
	0x03, 0x00, 0x00, 0x00, //0x002c
	0x68, 0x02, 0x04, 0x02, //0x0030
	0xff, 0x03, 0xe0, 0x03, //0x0034
	0x90, 0x2e, 0x20, 0x00, //0x0038
	0x01, 0x00, 0x37, 0x00, //0x003c
	0x00, 0x00, 0x00, 0x00, //0x0040
	0xa5, 0x00, 0x40, 0x00, //0x0044
	0x01, 0x50, 0x00, 0x00, //0x0048
	0x95, 0xcc, 0x7a, 0x07, //0x004c
	0x08, 0x08, 0x08, 0x08, //0x0050
	0x10, 0x08, 0x08, 0x08, //0x0054
	0x08, 0x00, 0x00, 0x00, //0x0058
	0xcc, 0x05, 0x90, 0x2e, //0x005c
	0x40, 0x00, 0x7b, 0x00, //0x0060
	0xf4, 0xb1, 0x07, 0x00, //0x0064
	0x7b, 0x00, 0x5f, 0x00, //0x0068
	0x00, 0x00, 0x00, 0x00, //0x006c
	0x02, 0x00, 0x04, 0x00, //0x0070
	0x40, 0x08, 0x2d, 0x12, //0x0074
	0xf1, 0x09, 0x75, 0x0e, //0x0078
	0x20, 0x0c, 0x97, 0x0a, //0x007c
	0x5f, 0x0f, 0xf3, 0x09, //0x0080
	0x0c, 0x00, 0x0a, 0xe1, //0x0084
	0x33, 0x00, 0x33, 0x00, //0x0088
	0x33, 0x00, 0x33, 0x00, //0x008c
	0x4a, 0xd4, 0x00, 0x04, //0x0090
	0xa8, 0x0c, 0x96, 0x0b, //0x0094
	0x8c, 0x08, 0x93, 0x09, //0x0098
	0x1d, 0x20, 0x03, 0x00, //0x009c
	0x53, 0xed, 0x38, 0x15, //0x00a0
	0x6e, 0xee, 0xe5, 0x26, //0x00a4
	0x20, 0xb2, 0xb5, 0x24, //0x00a8
	0x6c, 0x2d, 0xa9, 0x19, //0x00ac
	0xfe, 0xfb, 0x0f, 0x80, //0x00b0
	0x00, 0x00, 0x00, 0x00, //0x00b4
	0x00, 0x00, 0x00, 0x00, //0x00b8
	0x00, 0x00, 0x00, 0x00, //0x00bc
	0x00, 0x00, 0x00, 0x00, //0x00c0
	0x00, 0x00, 0x00, 0x00, //0x00c4
	0x00, 0x02, 0x9a, 0x00, //0x00c8
	0x66, 0x1f, 0xcd, 0x1f, //0x00cc
	0x33, 0x03, 0x00, 0x1f, //0x00d0
	0x00, 0x00, 0x9a, 0x1f, //0x00d4
	0x66, 0x02, 0x00, 0x00, //0x00d8
	0x00, 0x00, 0x00, 0x00, //0x00dc
	0x00, 0x02, 0x9a, 0x00, //0x00e0
	0x66, 0x1f, 0xcd, 0x1f, //0x00e4
	0x33, 0x03, 0x00, 0x1f, //0x00e8
	0x00, 0x00, 0x9a, 0x1f, //0x00ec
	0x66, 0x02, 0x00, 0x20, //0x00f0
	0x00, 0x00, 0x00, 0x00, //0x00f4
	0x00, 0x02, 0x00, 0x00, //0x00f8
	0x00, 0x00, 0xcd, 0x1f, //0x00fc
	0xcd, 0x02, 0x66, 0x1f, //0x0100
	0x00, 0x00, 0x9a, 0x1f, //0x0104
	0x66, 0x02, 0x00, 0x00, //0x0108
	0x00, 0x00, 0x00, 0x00, //0x010c
	0x00, 0x02, 0x00, 0x00, //0x0110
	0x00, 0x00, 0xcd, 0x1f, //0x0114
	0xcd, 0x02, 0x66, 0x1f, //0x0118
	0x00, 0x00, 0x9a, 0x1f, //0x011c
	0x66, 0x02, 0x00, 0x00, //0x0120
	0x00, 0x00, 0x00, 0x00, //0x0124
	0x00, 0x02, 0x00, 0x00, //0x0128
	0x00, 0x00, 0xcf, 0x1f, //0x012c
	0xcd, 0x02, 0x68, 0x1f, //0x0130
	0x00, 0x00, 0x9c, 0x1f, //0x0134
	0x66, 0x02, 0x00, 0x00, //0x0138
	0x00, 0x00, 0x00, 0x00, //0x013c
	0x01, 0x00, 0x30, 0x00, //0x0140
	0x30, 0x00, 0x30, 0x00, //0x0144
	0x30, 0x00, 0x30, 0x00, //0x0148
	0x30, 0x00, 0x30, 0x00, //0x014c
	0x30, 0x00, 0x30, 0x00, //0x0150
	0x30, 0x00, 0x30, 0x00, //0x0154
	0x30, 0x00, 0x30, 0x00, //0x0158
	0x00, 0x00, 0x00, 0x00, //0x015c
	0x00, 0x00, 0x00, 0x00, //0x0160
	0x00, 0x00, 0x00, 0x00, //0x0164
	0x00, 0x00, 0x00, 0x00, //0x0168
	0x00, 0x00, 0xaa, 0x02, //0x016c
	0x0d, 0x00, 0x00, 0xff, //0x0170
	0x00, 0x00, 0x01, 0x02, //0x0174
	0x03, 0x03, 0x03, 0x03, //0x0178
	0x03, 0x03, 0x03, 0x03, //0x017c
	0x00, 0x00, 0x00, 0x00, //0x0180
	0x00, 0x00, 0x00, 0x00, //0x0184
	0x00, 0x00, 0x00, 0x00, //0x0188
	0x00, 0x00, 0x00, 0x00, //0x018c
	0x00, 0x00, 0x00, 0x00, //0x0190
	0x00, 0x00, 0x00, 0x00, //0x0194
	0x00, 0x00, 0x00, 0x00, //0x0198
	0x00, 0x00, 0x00, 0x00, //0x019c
	0x00, 0x00, 0x00, 0x00, //0x01a0
	0x01, 0x00, 0x00, 0x00, //0x01a4
	0x45, 0x00, 0x01, 0x10, //0x01a8
	0x00, 0xf1, 0x00, 0x00, //0x01ac
	0x00, 0x00, 0x00, 0x00, //0x01b0
	0x07, 0x07, 0x07, 0x07, //0x01b4
	0x06, 0x06, 0x06, 0x06, //0x01b8
	0x05, 0x05, 0x05, 0x05, //0x01bc
	0x01, 0x00, 0x00, 0x00, //0x01c0
	0x03, 0x00, 0x02, 0x00, //0x01c4
	0x00, 0x08, 0x00, 0x00, //0x01c8
	0x01, 0x01, 0x02, 0x02, //0x01cc
	0x02, 0x03, 0x03, 0x02, //0x01d0
	0x05, 0x04, 0x05, 0x06, //0x01d4
	0x00, 0x00, 0x00, 0x00, //0x01d8
	0x00, 0x00, 0x00, 0x00, //0x01dc
	0x00, 0x00, 0x00, 0x00, //0x01e0
	0x00, 0x00, 0x00, 0x00, //0x01e4
	0x00, 0x00, 0x00, 0x00, //0x01e8
	0x00, 0x00, 0x00, 0x00, //0x01ec
	0x00, 0x00, 0x00, 0x00, //0x01f0
	0x00, 0x00, 0x00, 0x00, //0x01f4
	0x00, 0x00, 0x00, 0x00, //0x01f8
	0x00, 0x00, 0x00, 0x00, //0x01fc
	0x00, 0x00, 0x00, 0x00, //0x0200
	0x00, 0x00, 0x00, 0x00, //0x0204
	0x00, 0x00, 0x00, 0x00, //0x0208
	0x00, 0x00, 0x00, 0x00, //0x020c
	0x00, 0x00, 0x00, 0x00, //0x0210
	0x00, 0x00, 0x00, 0x00, //0x0214
	0x00, 0x00, 0x00, 0x00, //0x0218
	0x00, 0x00, 0x00, 0x00, //0x021c
	0x00, 0x00, 0x00, 0x00, //0x0220
	0x00, 0x00, 0x00, 0x00, //0x0224
	0x00, 0x00, 0x00, 0x00, //0x0228
	0x00, 0x00, 0x00, 0x00, //0x022c
	0x00, 0x00, 0x00, 0x00, //0x0230
	0x00, 0x00, 0x00, 0x00, //0x0234
	0x00, 0x00, 0x00, 0x00, //0x0238
	0x00, 0x00, 0x00, 0x00, //0x023c
	0x00, 0x00, 0x00, 0x00, //0x0240
	0x00, 0x00, 0x00, 0x00, //0x0244
	0x00, 0x00, 0x00, 0x00, //0x0248
	0x00, 0x00, 0x00, 0x00, //0x024c
	0x00, 0x00, 0x00, 0x00, //0x0250
	0x00, 0x00, 0x00, 0x00, //0x0254
	0x00, 0x00, 0x00, 0x00, //0x0258
	0x00, 0x00, 0x00, 0x00, //0x025c
	0x00, 0x00, 0x00, 0x00, //0x0260
	0x00, 0x00, 0x00, 0x00, //0x0264
	0x00, 0x00, 0x00, 0x00, //0x0268
	0x00, 0x00, 0x00, 0x00, //0x026c
	0x00, 0x00, 0x00, 0x00, //0x0270
	0x00, 0x00, 0x00, 0x00, //0x0274
	0x00, 0x00, 0x00, 0x00, //0x0278
	0x00, 0x00, 0x00, 0x00, //0x027c
	0x00, 0x00, 0x00, 0x00, //0x0280
	0x00, 0x00, 0x00, 0x00, //0x0284
	0x00, 0x00, 0x00, 0x00, //0x0288
	0x00, 0x00, 0x00, 0x00, //0x028c
	0x00, 0x00, 0x00, 0x00, //0x0290
	0x00, 0x00, 0x00, 0x00, //0x0294
	0x00, 0x00, 0x00, 0x00, //0x0298
	0x00, 0x00, 0x00, 0x00, //0x029c
	0x00, 0x00, 0x00, 0x00, //0x02a0
	0x00, 0x00, 0x00, 0x00, //0x02a4
	0x00, 0x00, 0x00, 0x00, //0x02a8
	0x00, 0x00, 0x00, 0x00, //0x02ac
	0x00, 0x00, 0x00, 0x00, //0x02b0
	0x00, 0x00, 0x00, 0x00, //0x02b4
	0x00, 0x00, 0x00, 0x00, //0x02b8
	0x00, 0x00, 0x00, 0x00, //0x02bc
	0x00, 0x00, 0x00, 0x00, //0x02c0
	0x00, 0x00, 0x00, 0x00, //0x02c4
	0x00, 0x00, 0x00, 0x00, //0x02c8
	0x00, 0x00, 0x00, 0x00, //0x02cc
	0x00, 0x00, 0x00, 0x00, //0x02d0
	0x00, 0x00, 0x00, 0x00, //0x02d4
	0x00, 0x00, 0x00, 0x00, //0x02d8
	0x00, 0x00, 0x00, 0x00, //0x02dc
	0x00, 0x00, 0x00, 0x00, //0x02e0
	0x00, 0x00, 0x00, 0x00, //0x02e4
	0x00, 0x00, 0x00, 0x00, //0x02e8
	0x00, 0x00, 0x00, 0x00, //0x02ec
	0x00, 0x00, 0x00, 0x00, //0x02f0
	0x00, 0x00, 0x00, 0x00, //0x02f4
	0x00, 0x00, 0x00, 0x00, //0x02f8
	0x00, 0x00, 0x00, 0x00, //0x02fc
	0x00, 0x00, 0x00, 0x00, //0x0300
	0x00, 0x00, 0x00, 0x00, //0x0304
	0x00, 0x00, 0x00, 0x00, //0x0308
	0x00, 0x00, 0x00, 0x00, //0x030c
	0x00, 0x00, 0x00, 0x00, //0x0310
	0x00, 0x00, 0x00, 0x00, //0x0314
	0x00, 0x00, 0x00, 0x00, //0x0318
	0x00, 0x00, 0x00, 0x00, //0x031c
	0x00, 0x00, 0x00, 0x00, //0x0320
	0x00, 0x00, 0x00, 0x00, //0x0324
	0x00, 0x00, 0x00, 0x00, //0x0328
	0x00, 0x00, 0x00, 0x00, //0x032c
	0x00, 0x00, 0x00, 0x00, //0x0330
	0x00, 0x00, 0x00, 0x00, //0x0334
	0x00, 0x00, 0x00, 0x00, //0x0338
	0x00, 0x00, 0x00, 0x00, //0x033c
	0x00, 0x00, 0x00, 0x00, //0x0340
	0x00, 0x00, 0x00, 0x00, //0x0344
	0x00, 0x00, 0x00, 0x00, //0x0348
	0x00, 0x00, 0x00, 0x00, //0x034c
	0x00, 0x00, 0x00, 0x00, //0x0350
	0x00, 0x00, 0x00, 0x00, //0x0354
	0x00, 0x00, 0x00, 0x00, //0x0358
	0x00, 0x00, 0x00, 0x00, //0x035c
	0x00, 0x00, 0x00, 0x00, //0x0360
	0x00, 0x00, 0x00, 0x00, //0x0364
	0x00, 0x00, 0x00, 0x00, //0x0368
	0x00, 0x00, 0x00, 0x00, //0x036c
	0x00, 0x00, 0x00, 0x00, //0x0370
	0x00, 0x00, 0x00, 0x00, //0x0374
	0x00, 0x00, 0x00, 0x00, //0x0378
	0x00, 0x00, 0x00, 0x00, //0x037c
	0x00, 0x00, 0x00, 0x00, //0x0380
	0x00, 0x00, 0x00, 0x00, //0x0384
	0x00, 0x00, 0x00, 0x00, //0x0388
	0x00, 0x00, 0x00, 0x00, //0x038c
	0x00, 0x00, 0x00, 0x00, //0x0390
	0x00, 0x00, 0x00, 0x00, //0x0394
	0x00, 0x00, 0x00, 0x00, //0x0398
	0x00, 0x00, 0x00, 0x00, //0x039c
	0x00, 0x00, 0x00, 0x00, //0x03a0
	0x00, 0x00, 0x00, 0x00, //0x03a4
	0x00, 0x00, 0x00, 0x00, //0x03a8
	0x00, 0x00, 0x00, 0x00, //0x03ac
	0x00, 0x00, 0x00, 0x00, //0x03b0
	0x00, 0x00, 0x00, 0x00, //0x03b4
	0x00, 0x00, 0x00, 0x00, //0x03b8
	0x00, 0x00, 0x00, 0x00, //0x03bc
	0x00, 0x00, 0x00, 0x00, //0x03c0
	0x00, 0x00, 0x00, 0x00, //0x03c4
	0x00, 0x00, 0x00, 0x00, //0x03c8
	0x00, 0x00, 0x00, 0x00, //0x03cc
	0x00, 0x00, 0x00, 0x00, //0x03d0
	0x00, 0x00, 0x00, 0x00, //0x03d4
	0x00, 0x00, 0x00, 0x00, //0x03d8
	0x00, 0x00, 0xff, 0xff, //0x03dc
	0x05, 0x00, 0x00, 0x00, //0x03e0
	0x15, 0x00, 0x80, 0x00, //0x03e4
	0x10, 0x07, 0x26, 0x02, //0x03e8
	0x19, 0x01, 0x00, 0x00, //0x03ec
	0x1e, 0x2f, 0x0f, 0x00, //0x03f0
	0x19, 0x18, 0x18, 0x16, //0x03f4
	0x16, 0x16, 0x15, 0x14, //0x03f8
	0x11, 0x10, 0x10, 0x10, //0x03fc
	0x00, 0x00, 0x00, 0x00, //0x0400
	0x00, 0x00, 0x00, 0x00, //0x0404
	0x00, 0x00, 0x00, 0x00, //0x0408
	0x01, 0xa0, 0x40, 0x40, //0x040c
	0xc0, 0xc0, 0xa8, 0xa8, //0x0410
	0x88, 0x80, 0x60, 0x50, //0x0414
	0x40, 0x40, 0x40, 0x40, //0x0418
	0x01, 0x00, 0x00, 0x00, //0x041c
	0x30, 0x20, 0x10, 0x10, //0x0420
	0x00, 0x00, 0x80, 0x02, //0x0424
	0x30, 0x30, 0x30, 0x30, //0x0428
	0x30, 0x30, 0x20, 0x00, //0x042c
	0x00, 0x00, 0x00, 0x00, //0x0430
	0x20, 0x20, 0x20, 0x20, //0x0434
	0x20, 0x20, 0x10, 0x00, //0x0438
	0x00, 0x00, 0x00, 0x00, //0x043c
	0x10, 0x10, 0x10, 0x10, //0x0440
	0x10, 0x10, 0x10, 0x10, //0x0444
	0x10, 0x10, 0x10, 0x08, //0x0448
	0x10, 0x10, 0x10, 0x10, //0x044c
	0x10, 0x10, 0x10, 0x10, //0x0450
	0x10, 0x10, 0x10, 0x08, //0x0454
	0x00, 0x00, 0x00, 0x00, //0x0458
	0x00, 0x00, 0x00, 0x00, //0x045c
	0x00, 0x00, 0x00, 0x00, //0x0460
	0x00, 0x00, 0x00, 0x00, //0x0464
	0x00, 0x00, 0x00, 0x00, //0x0468
	0x00, 0x00, 0x00, 0x00, //0x046c
	0x80, 0x02, 0x80, 0x02, //0x0470
	0x80, 0x02, 0x80, 0x02, //0x0474
	0x80, 0x02, 0x80, 0x02, //0x0478
	0x80, 0x02, 0x80, 0x02, //0x047c
	0x80, 0x02, 0x80, 0x02, //0x0480
	0x80, 0x02, 0x80, 0x02, //0x0484
	0x60, 0x02, 0x00, 0x80, //0x0488
	0x60, 0x00, 0x00, 0x00, //0x048c
	0x60, 0x00, 0x00, 0x00, //0x0490
	0x00, 0x00, 0x00, 0x00, //0x0494
	0x00, 0x00, 0x00, 0x00, //0x0498
	0x00, 0x00, 0x00, 0x00, //0x049c
	0x00, 0x00, 0x00, 0x00, //0x04a0
	0x00, 0x00, 0x00, 0x00, //0x04a4
	0x00, 0x00, 0x00, 0x00, //0x04a8
	0x00, 0x00, 0x00, 0x00, //0x04ac
	0x00, 0x00, 0x00, 0x00, //0x04b0
	0x00, 0x00, 0x00, 0x00, //0x04b4
	0x00, 0x00, 0x00, 0x00, //0x04b8
	0x00, 0x00, 0x00, 0x00, //0x04bc
	0x00, 0x00, 0x00, 0x00, //0x04c0
	0x00, 0x00, 0x00, 0x00, //0x04c4
	0x00, 0x00, 0x00, 0x00, //0x04c8
	0x00, 0x00, 0x00, 0x00, //0x04cc
	0x00, 0x00, 0x00, 0x00, //0x04d0
	0x00, 0x00, 0x00, 0x00, //0x04d4
	0x00, 0x00, 0x00, 0x00, //0x04d8
	0x00, 0x00, 0x00, 0x00, //0x04dc
	0x00, 0x00, 0x00, 0x00, //0x04e0
	0x00, 0x00, 0x00, 0x00, //0x04e4
	0x00, 0x00, 0x00, 0x00, //0x04e8
	0x00, 0x00, 0x00, 0x00, //0x04ec
	0x00, 0x00, 0x00, 0x00, //0x04f0
	0x00, 0x00, 0x00, 0x00, //0x04f4
	0x00, 0x00, 0x00, 0x00, //0x04f8
	0x00, 0x00, 0x00, 0x00, //0x04fc
	0x00, 0x00, 0x00, 0x00, //0x0500
	0x00, 0x00, 0x00, 0x00, //0x0504
	0x00, 0x00, 0x00, 0x00, //0x0508
	0x00, 0x00, 0x00, 0x00, //0x050c
	0x00, 0x00, 0x00, 0x00, //0x0510
	0x00, 0x00, 0x00, 0x00, //0x0514
	0x00, 0x00, 0x00, 0x00, //0x0518
	0x00, 0x00, 0x00, 0x00, //0x051c
	0x00, 0x00, 0x00, 0x00, //0x0520
	0x00, 0x00, 0x00, 0x00, //0x0524
	0x00, 0x00, 0x00, 0x00, //0x0528
	0x00, 0x00, 0x00, 0x00, //0x052c
	0x00, 0x00, 0x00, 0x00, //0x0530
	0x00, 0x00, 0x00, 0x00, //0x0534
	0x00, 0x00, 0x0c, 0x00, //0x0538
	0x16, 0x00, 0x20, 0x00, //0x053c
	0x2c, 0x00, 0x36, 0x00, //0x0540
	0x42, 0x00, 0x4c, 0x00, //0x0544
	0x58, 0x00, 0x62, 0x00, //0x0548
	0x6c, 0x00, 0x77, 0x00, //0x054c
	0x82, 0x00, 0x8d, 0x00, //0x0550
	0x97, 0x00, 0xa1, 0x00, //0x0554
	0xab, 0x00, 0xb3, 0x00, //0x0558
	0xbd, 0x00, 0xc7, 0x00, //0x055c
	0xcf, 0x00, 0xd9, 0x00, //0x0560
	0xe3, 0x00, 0xeb, 0x00, //0x0564
	0xf4, 0x00, 0xfe, 0x00, //0x0568
	0x06, 0x01, 0x0e, 0x01, //0x056c
	0x16, 0x01, 0x1e, 0x01, //0x0570
	0x26, 0x01, 0x2e, 0x01, //0x0574
	0x36, 0x01, 0x53, 0x01, //0x0578
	0x6f, 0x01, 0x87, 0x01, //0x057c
	0x9f, 0x01, 0xb6, 0x01, //0x0580
	0xca, 0x01, 0xde, 0x01, //0x0584
	0xf1, 0x01, 0x03, 0x02, //0x0588
	0x13, 0x02, 0x23, 0x02, //0x058c
	0x32, 0x02, 0x40, 0x02, //0x0590
	0x4c, 0x02, 0x59, 0x02, //0x0594
	0x65, 0x02, 0x71, 0x02, //0x0598
	0x7b, 0x02, 0x85, 0x02, //0x059c
	0x8f, 0x02, 0x99, 0x02, //0x05a0
	0xa2, 0x02, 0xaa, 0x02, //0x05a4
	0xb2, 0x02, 0xd1, 0x02, //0x05a8
	0xeb, 0x02, 0x03, 0x03, //0x05ac
	0x19, 0x03, 0x2c, 0x03, //0x05b0
	0x3e, 0x03, 0x4f, 0x03, //0x05b4
	0x5e, 0x03, 0x6c, 0x03, //0x05b8
	0x7a, 0x03, 0x87, 0x03, //0x05bc
	0x93, 0x03, 0x9e, 0x03, //0x05c0
	0xa9, 0x03, 0xb4, 0x03, //0x05c4
	0xbe, 0x03, 0xc8, 0x03, //0x05c8
	0xd1, 0x03, 0xda, 0x03, //0x05cc
	0xe2, 0x03, 0xea, 0x03, //0x05d0
	0xf1, 0x03, 0xf9, 0x03, //0x05d4
	0xff, 0x03, 0x00, 0x00, //0x05d8
	0x00, 0x00, 0x02, 0x00, //0x05dc
	0x04, 0x00, 0x06, 0x00, //0x05e0
	0x08, 0x00, 0x0a, 0x00, //0x05e4
	0x0c, 0x00, 0x0e, 0x00, //0x05e8
	0x10, 0x00, 0x12, 0x00, //0x05ec
	0x14, 0x00, 0x16, 0x00, //0x05f0
	0x18, 0x00, 0x1a, 0x00, //0x05f4
	0x1c, 0x00, 0x1e, 0x00, //0x05f8
	0x20, 0x00, 0x22, 0x00, //0x05fc
	0x24, 0x00, 0x26, 0x00, //0x0600
	0x28, 0x00, 0x2a, 0x00, //0x0604
	0x2c, 0x00, 0x2e, 0x00, //0x0608
	0x30, 0x00, 0x32, 0x00, //0x060c
	0x34, 0x00, 0x36, 0x00, //0x0610
	0x38, 0x00, 0x3a, 0x00, //0x0614
	0x3c, 0x00, 0x3e, 0x00, //0x0618
	0x40, 0x00, 0x48, 0x00, //0x061c
	0x50, 0x00, 0x58, 0x00, //0x0620
	0x60, 0x00, 0x68, 0x00, //0x0624
	0x70, 0x00, 0x78, 0x00, //0x0628
	0x80, 0x00, 0x88, 0x00, //0x062c
	0x90, 0x00, 0x98, 0x00, //0x0630
	0xa0, 0x00, 0xa8, 0x00, //0x0634
	0xb0, 0x00, 0xb8, 0x00, //0x0638
	0xc0, 0x00, 0xc8, 0x00, //0x063c
	0xd0, 0x00, 0xd8, 0x00, //0x0640
	0xe0, 0x00, 0xe8, 0x00, //0x0644
	0xf0, 0x00, 0xf8, 0x00, //0x0648
	0x00, 0x01, 0x20, 0x01, //0x064c
	0x40, 0x01, 0x60, 0x01, //0x0650
	0x80, 0x01, 0xa0, 0x01, //0x0654
	0xc0, 0x01, 0xe0, 0x01, //0x0658
	0x00, 0x02, 0x20, 0x02, //0x065c
	0x40, 0x02, 0x60, 0x02, //0x0660
	0x80, 0x02, 0xa0, 0x02, //0x0664
	0xc0, 0x02, 0xe0, 0x02, //0x0668
	0x00, 0x03, 0x20, 0x03, //0x066c
	0x40, 0x03, 0x60, 0x03, //0x0670
	0x80, 0x03, 0xa0, 0x03, //0x0674
	0xc0, 0x03, 0xe0, 0x03, //0x0678
	0xff, 0x03, 0x00, 0x00, //0x067c
	0x00, 0x00, 0x3c, 0x00, //0x0680
	0x52, 0x00, 0x63, 0x00, //0x0684
	0x70, 0x00, 0x7c, 0x00, //0x0688
	0x87, 0x00, 0x91, 0x00, //0x068c
	0x9a, 0x00, 0xa3, 0x00, //0x0690
	0xab, 0x00, 0xb2, 0x00, //0x0694
	0xba, 0x00, 0xc0, 0x00, //0x0698
	0xc7, 0x00, 0xcd, 0x00, //0x069c
	0xd4, 0x00, 0xd9, 0x00, //0x06a0
	0xdf, 0x00, 0xe5, 0x00, //0x06a4
	0xea, 0x00, 0xef, 0x00, //0x06a8
	0xf5, 0x00, 0xfa, 0x00, //0x06ac
	0xfe, 0x00, 0x03, 0x01, //0x06b0
	0x08, 0x01, 0x0c, 0x01, //0x06b4
	0x11, 0x01, 0x15, 0x01, //0x06b8
	0x1a, 0x01, 0x1e, 0x01, //0x06bc
	0x22, 0x01, 0x32, 0x01, //0x06c0
	0x41, 0x01, 0x4f, 0x01, //0x06c4
	0x5d, 0x01, 0x6a, 0x01, //0x06c8
	0x76, 0x01, 0x82, 0x01, //0x06cc
	0x8e, 0x01, 0x99, 0x01, //0x06d0
	0xa3, 0x01, 0xae, 0x01, //0x06d4
	0xb8, 0x01, 0xc2, 0x01, //0x06d8
	0xcb, 0x01, 0xd5, 0x01, //0x06dc
	0xde, 0x01, 0xe7, 0x01, //0x06e0
	0xf0, 0x01, 0xf8, 0x01, //0x06e4
	0x01, 0x02, 0x09, 0x02, //0x06e8
	0x11, 0x02, 0x19, 0x02, //0x06ec
	0x21, 0x02, 0x3f, 0x02, //0x06f0
	0x5b, 0x02, 0x76, 0x02, //0x06f4
	0x8f, 0x02, 0xa7, 0x02, //0x06f8
	0xbf, 0x02, 0xd5, 0x02, //0x06fc
	0xeb, 0x02, 0x00, 0x03, //0x0700
	0x14, 0x03, 0x27, 0x03, //0x0704
	0x3b, 0x03, 0x4d, 0x03, //0x0708
	0x5f, 0x03, 0x71, 0x03, //0x070c
	0x82, 0x03, 0x93, 0x03, //0x0710
	0xa3, 0x03, 0xb3, 0x03, //0x0714
	0xc3, 0x03, 0xd3, 0x03, //0x0718
	0xe2, 0x03, 0xf1, 0x03, //0x071c
	0xff, 0x03, 0x00, 0x00, //0x0720
	0x00, 0x00, 0x60, 0x00, //0x0724
	0x00, 0x00, 0x00, 0x00, //0x0728
	0x00, 0x00, 0x00, 0x00, //0x072c
	0x00, 0x00, 0x00, 0x00, //0x0730
	0x60, 0x60, 0x60, 0x60, //0x0734
	0x60, 0x60, 0x60, 0x60, //0x0738
	0x60, 0x60, 0x60, 0x60, //0x073c
	0x20, 0x20, 0x20, 0x20, //0x0740
	0x20, 0x20, 0x20, 0x20, //0x0744
	0x20, 0x20, 0x20, 0x20, //0x0748
	0x01, 0x00, 0x00, 0x00, //0x074c
	0x00, 0x00, 0x10, 0x10, //0x0750
	0x10, 0x10, 0x10, 0x10, //0x0754
	0x10, 0x10, 0x10, 0x10, //0x0758
	0x00, 0x10, 0x00, 0x00, //0x075c
};

#endif
