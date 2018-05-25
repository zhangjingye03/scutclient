#include "info.h"
#include "tracelog.h"

struct in_addr udpserver_ipaddr;
struct in_addr dns_ipaddr;
struct in_addr my_ipaddr;
char userSpecifiedIp = 0;
char *UserName;
char *Password;
char *HookCmd;
char DeviceName[IFNAMSIZ] = "eth0";
char HostName[32];
char *Hash = "2ec15ad258aee9604b18f2f8114da38db16efd00";
unsigned char Version[64] = { 0x44, 0x72, 0x43, 0x4f, 0x4d, 0x00, 0x96, 0x02, 0x2a };
int Version_len = 9;

void hexStrToByte(const char* source, unsigned char* dest, int sourceLen) {
	short i;
	unsigned char highByte, lowByte;

	for (i = 0; i < sourceLen; i += 2) {
		highByte = toupper(source[i]);
		lowByte = toupper(source[i + 1]);

		if (highByte > 0x39) {
			highByte -= 0x37;
		} else {
			highByte -= 0x30;
		}

		if (lowByte > 0x39) {
			lowByte -= 0x37;
		} else {
			lowByte -= 0x30;
		}

		dest[i / 2] = (highByte << 4) | lowByte;
	}
	return;
}
