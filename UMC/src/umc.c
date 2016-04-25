#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include "socketServer.h"
#include <commons/collections/list.h>

struct server{
	int socketServer;
	struct sockaddr_in direccion;
	t_list* listaSockets;
};

int main(void) {
	struct server serverUMC;
	serverUMC = crearServer(9000);
	ponerServerEscuchaSelect(serverUMC);
	return 0;
}
