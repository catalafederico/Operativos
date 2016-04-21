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
#include <commons/log.h>
#include "basicfunciones.h"

#define PUERTONEWCONX 26987




int main(int argc, char **argv) {

	int umcsocket, othersocket; // Escucha sobre socketumc, nuevas conexiones sobre new_df
	struct sockaddr_in umc_adress; //datos sobre mi direccion
	struct sockaddr_in other_adress; // datos de direcciones entrantes
	t_log* umc_infol = log_create("log_umc_socket", "umc", false, LOG_LEVEL_INFO);
	//t_log* umc_warningl = log_create("log_umc_socket", "umc", false, LOG_LEVEL_WARNING);

	crearSocket(&umcsocket);
	log_info(umc_infol,"Socket creado correctamente");

	umc_adress.sin_family = AF_INET;
	umc_adress.sin_port = htons(PUERTONEWCONX);
	umc_adress.sin_addr.s_addr = INADDR_ANY;
	memset(&(umc_adress.sin_zero),'\0',8);

	abrirPuerto(umcsocket,&umc_adress);
	log_info(umc_infol,"Socket creado correctamente");

	log_info(umc_infol,"Se pone socket a la escucha de nuevas conexiones");
	escucharConexiones(umcsocket,5);
	log_info(umc_infol,"Se escucha una conexion");

	while(true){
		log_info(umc_infol,"Estableciendo conexion");
		aceptarConexion(&othersocket,umcsocket,&other_adress);
		log_info(umc_infol,"Conexion establecida");

        if (!fork()) {
            close(umcsocket);
            if (send(othersocket, "Hello, world!\n", 14, 0) == -1)
                perror("send");
            close(othersocket);
            exit(0);
        }
        close(othersocket);
	}

return 0;
}
