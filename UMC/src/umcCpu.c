/*
 * umcNucleo.c
 *
 *  Created on: 13/5/2016
 *      Author: utnso
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <sockets/header.h>
#include <sockets/basicFunciones.h>
#include "archivoConf.h"
#include "estructurasUMC.h"

void* conexionCpu(tempStruct socketCpu){
	while(1){
		int* header =(int *) recibirStream(socketCpu.socket,sizeof(int));
		switch (*header) {
			case FINALIZACIONPROGRAMA:
				break;
			case NUEVOPROGRAMA:
				inicializar_programa(socketCpu);
				break;
			default:
				break;
		}
	}
}



