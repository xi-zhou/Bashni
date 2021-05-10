#include "global.h"
#include "config.h"
#include "connector.h"

//int	msg_length;

//char *client_message;
int	socket_desc;
int	dflag;


int
initiateConnection()
{
    config conf = readConfig();

	//int socket_desc;
	struct addrinfo hints, *res0;
	int retcode;
	//get IP for server and put into addrinfo strukt res0
	//getaddrinfo("sysprak.priv.lab.nm.ifi.lmu.de", "1357", NULL, &res0);

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	retcode = getaddrinfo(conf.hostname, conf.portnumber, &hints, &res0);
	if (retcode) {
		errx(1, "%s", gai_strerror(retcode));
	}

	//Create socket
	socket_desc = -1;
	if (dflag == 1) {
		int test_var = res0->ai_socktype;
		printf("socktype: %i\n", test_var);
	}
	socket_desc = socket(res0->ai_family, res0->ai_socktype, 0);

	if (socket_desc == -1) {
		errx(1, "Error due to socket.");
	}

	//Connect to server
	if (connect(socket_desc , res0->ai_addr, res0->ai_addrlen) < 0)
	{
		errx(1, "connect error.");
		return 1;
	}

	puts("Connected\n");


	freeaddrinfo(res0);
    freeConfig(conf);

	return 0;
}
