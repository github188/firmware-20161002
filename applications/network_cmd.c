#include <finsh.h>
#include <stdio.h>
#include <stdlib.h>

int dhcpd(int argc, char** argv)
{
	if (argc != 2)
	{
		printf("dhcpd netif\n");
		return 0;
	}

	dhcpd_start(argv[1]);
	return 0;
}
MSH_CMD_EXPORT(dhcpd, start dhcpd on network interface);

int wifi_ap(void)
{
	/* Wi-Fi AP and start mjpeg */
	wifi_test(1, "iot", "iotcamera");
	set_if("ap", "192.168.169.1", "192.168.169.1", "255.255.255.0");
	dhcpd_start("ap");
	mjpeg(80);

	return 0;
}
MSH_CMD_EXPORT(wifi_ap, start wifi ap and start mjpeg);
