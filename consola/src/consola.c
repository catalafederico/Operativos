/*
 ============================================================================
 Name        : consola.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>








int main(void) {




	struct sockaddr_in direccionServidor;
	direccionServidor.sin_family = AF_INET;
	direccionServidor.sin_addr.s_addr = INADDR_ANY;
	direccionServidor.sin_port = htons(5000);
	/*struct sockaddr_in nucleo_addr_proc;
			nucleo_addr_proc.sin_family = AF_INET;
			nucleo_addr_proc.sin_addr.s_addr = INADDR_ANY;
			nucleo_addr_proc.sin_port = htons(5000);*/
	/*	int cliente = socket(AF_INET, SOCK_STREAM, 0);
		if (connect(cliente, (void*) &direccionServidor, sizeof(direccionServidor)) != 0) {
			perror("No se pudo conectar");
			return 1;
		}*/
	struct sockaddr_in nucleo_addr_proc;
		nucleo_addr_proc.sin_family = AF_INET;
		nucleo_addr_proc.sin_addr.s_addr = INADDR_ANY;
		nucleo_addr_proc.sin_port = htons(5000);


		int cliente = socket(AF_INET, SOCK_STREAM, 0);
			if (connect(cliente, (void*) &nucleo_addr_proc, sizeof(nucleo_addr_proc)) != 0) {
				perror("No se pudo conectar");
				return 1;}

	while (1) {
		char mensaje[SOMAXCONN];
		scanf("%s", mensaje);

		send(cliente, mensaje, strlen(mensaje), 0);
	}

	return 0;
}




