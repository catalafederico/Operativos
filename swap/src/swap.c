/*
 ============================================================================
 Name        : swap.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <commons/config.h>
#include <commons/log.h>
#include <sockets/socketServer.h>
#include <sockets/basicFunciones.h>
#include <commons/collections/list.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include "bitMap.h"
#include "archivoConfig.h"

//Variables globales

t_reg_config swap_configuracion;

int socketAdministradorDeMemoria;

int *bitMap;

proceso* listaSwap;

pthread_mutex_t mutex;

t_log* logguerSwap;


int main(void) {
	logguerSwap = log_create("../log_swap", "Swap", false, LOG_LEVEL_INFO);
	//creo semaforo
	pthread_mutex_init(&mutex,NULL);
	//cargo patametro
	get_config_params();
	//bitmap para organizacion de la particion swap
	inicializarBitMap();
	//Creo particion
    crearArchivo();
    //listaSwap = NULL;
    //Escucho por la umc
	struct server servidor;
	servidor = crearServer(swap_configuracion.PUERTO_ESCUCHA);
	log_info(logguerSwap, "Escuchando conexion UMC.");
	ponerServerEscucha(servidor);
	struct sockaddr_in direccionEntrante;
	aceptarConexion(&socketAdministradorDeMemoria,servidor.socketServer,&direccionEntrante);
	printf("Conexion UMC establecida\n");
	log_info(logguerSwap, "Conexion UMC establecida");
	//Espero indicaciones de la umc
	int seguir = 1;
	while(seguir){
		int* header = leerHeader(socketAdministradorDeMemoria);
		switch(*header){
		        case 50:
					iniciar();
				break;
				case 51:
					finalizar();
				break;
				case 52:
					usleep(swap_configuracion.RETARDO_ACCESO*1000);
					leer();
				break;
				case 53:
					usleep(swap_configuracion.RETARDO_ACCESO*1000);
					escribir();
				break;
				case -1://pierde conexion
					printf("Desconectado de UMC\n");
					seguir =0;
				break;
		}
		free(header);
	}
	log_info(logguerSwap, "Conexion UMC perdida, cerrando SWAP");
	free(bitMap);
    //Liberar bien ista wap ya q libera solo el primer puntero
    free(listaSwap);
	return 0;
}

void dormir(){
	usleep(swap_configuracion.RETARDO_COMPACTACION*1000);
}

void testEscribirEnPaginas() {
	escribirPagina(2, "1234567890");
	escribirPagina(3, "dadadadada");
	escribirPagina(0, "12345678910111213");
	escribirPagina(1,
			"dsnaudsabifbasifasifhasufausbfasupfdhusafphsdauensayfgesyafo");
	escribirPagina(0,
			"hola                                                                                                                                                                                                                                                           F");
	escribirPagina(0, "12345678910111213");
	escribirPagina(1, "como andas");
}

void testLeerPagina() {
	char* texto = leerPagina(0);
	char* texto2 = leerPagina(1);
	char* texto3 = leerPagina(2);
	char* texto4 = leerPagina(3);
	printf("%s\n", texto);
	printf("%s\n", texto2);
	printf("%s\n", texto3);
	printf("%s\n", texto4);
}
