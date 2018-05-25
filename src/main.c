/* File: main.c
 * ------------
 * 校园网802.1X客户端命令行
 */
#include "auth.h"
#include "info.h"
#include "tracelog.h"
#include <signal.h>

struct sigaction sa_term;

// 命令行参数列表
static const struct option long_options[] = {
	{"username", required_argument, NULL, 'u'},
	{"password", required_argument, NULL, 'p'},
	{"ip", required_argument, NULL, 'i'},
	{"iface", required_argument, NULL, 'f'},
	{"dns", required_argument, NULL, 'n'},
	{"hostname", required_argument, NULL, 't'},
	{"udp-server", required_argument, NULL, 's'},
	{"cli-version", required_argument,NULL, 'c'},
	{"net-time", required_argument,NULL, 'T'},
	{"hash", required_argument, NULL, 'h'},
	{"auth-exec", required_argument, NULL, 'E'},
	{"debug", optional_argument, NULL, 'D'},
	{"logoff", no_argument, NULL, 'o'},
	{NULL, no_argument, NULL, 0}
};

void PrintHelp(const char * argn) {
	printf("Usage: %s --username <username> --password <password> [options...]\n"
		" -f, --iface <ifname> Interface to perform authentication.\n"
		" -i, --ip <ipaddr> User specified IP address to submit to 802.1X server. \n"
		" -n, --dns <dns> DNS server address to be sent to UDP server.\n"
		" -t, --hostname <hostname>\n"
		" -s, --udp-server <server>\n"
		" -c, --cli-version <client version>\n"
		" -h, --hash <hash> DrAuthSvr.dll hash value.\n"
		" -E, --auth-exec <command> Command to be execute after EAP authentication success.\n"
		" -D, --debug\n"
		" -o, --logoff\n",
		argn);
}

void handle_term(int signal) {
	LogWrite(ALL, INF, "Exiting...");
	auth_8021x_Logoff();
	exit(0);
}

int main(int argc, char *argv[]) {
	int client = 1;
	int ch, tmpdbg;
	uint8_t a_hour = 255, a_minute = 255;
	uint8_t buf[128];
	int ret;
	time_t ctime;
	struct tm * cltime;

	while ((ch = getopt_long(argc, argv, "u:p:E:f:m:a:k:g:n:t:T:s:i:c:h:oD::",
			long_options, NULL)) != -1) {
		switch (ch) {
		case 'u':
			UserName = optarg;
			break;
		case 'p':
			Password = optarg;
			break;
		case 'E':
			HookCmd = optarg;
			break;
		case 'f':
			strcpy(DeviceName, optarg);
			break;
		case 'n':
			if (!inet_aton(optarg, &dns_ipaddr)) {
				LogWrite(INIT, ERROR, "DNS invalid!");
				exit(-1);
			}
			break;
		case 't':
			strcpy(HostName, optarg);
			break;
		case 's':
			if (!inet_aton(optarg, &udpserver_ipaddr)) {
				LogWrite(INIT, ERROR, "UDP server IP invalid!");
				exit(-1);
			}
			break;
		case 'i':
			if (!inet_aton(optarg, &my_ipaddr)) {
				LogWrite(INIT, ERROR, "User specified IP invalid!");
				exit(-1);
			}
			userSpecifiedIp = 1;
			break;
		case 'T':
			if((sscanf(optarg, "%hhu:%hhu", &a_hour, &a_minute) != 2) || (a_hour >= 24) || (a_minute >= 60)) {
				LogWrite(INIT, ERROR, "Time invalid!");
				exit(-1);
			}
			break;
		case 'c':
			hexStrToByte(optarg, buf, strlen(optarg));
			Version_len = strlen(optarg) / 2;
			memcpy(Version, buf, Version_len);
			break;
		case 'h':
			Hash = optarg;
			break;
		case 'D':
			if (optarg) {
				tmpdbg = atoi(optarg);
				if ((tmpdbg < NONE) || (tmpdbg > TRACE)) {
					LogWrite(INIT, ERROR, "Invalid debug level!");
				} else {
					cloglev = tmpdbg;
				}
			} else {
				cloglev = DEBUG;
			}
			break;
		case 'o':
			client = LOGOFF;
			break;
		default:
			PrintHelp(argv[0]);
			exit(-1);
			break;
		}
	}

	LogWrite(ALL, INF, "##################################");
	LogWrite(ALL, INF, "Powered by Scutclient Project");
	LogWrite(ALL, INF, "Contact us with QQ group 262939451");
	LogWrite(ALL, INF, "##################################");

	if (HostName[0] == 0)
		gethostname(HostName, sizeof(HostName));

	if ((client != LOGOFF) && !((UserName && Password && UserName[0] && Password[0]))) {
		LogWrite(INIT, ERROR, "Please specify username and password!");
		exit(-1);
	}
	if (udpserver_ipaddr.s_addr == 0)
		inet_aton(SERVER_ADDR, &udpserver_ipaddr);
	if (dns_ipaddr.s_addr == 0)
		inet_aton(DNS_ADDR, &dns_ipaddr);

	/* 配置退出登录的signal handler */
	sa_term.sa_handler = &handle_term;
	sa_term.sa_flags = SA_RESETHAND;
	sigfillset(&sa_term.sa_mask);
	sigaction(SIGTERM, &sa_term, NULL);
	sigaction(SIGINT, &sa_term, NULL);

	/* 调用子函数完成802.1X认证 */
	while(1) {
		ret = Authentication(client);
		if(ret == 1) {
			LogWrite(ALL, INF, "Restart authentication.");
		} else if(timeNotAllowed && (a_minute < 60)) {
			timeNotAllowed = 0;
			ctime = time(NULL);
			cltime = localtime(&ctime);
			if(((int)a_hour * 60 + a_minute) > ((int)(cltime -> tm_hour) * 60 + cltime -> tm_min)) {
				LogWrite(ALL, INF, "Waiting till %02hhd:%02hhd. Have a good sleep...", a_hour, a_minute);
				sleep((((int)a_hour * 60 + a_minute) - ((int)(cltime -> tm_hour) * 60 + cltime -> tm_min)) * 60 - cltime -> tm_sec);
			} else {
				break;
			}
		} else {
			break;
		}
	}
	LogWrite(ALL, ERROR, "Exit.");
	return 0;
}
