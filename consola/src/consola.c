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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <commons/config.h>
#include </home/utnso/workspace/tp-2016-1c-Explosive-code/bibliotecaC/socketCliente.h>






int main(void) {

	struct cliente clienteConsola;
	clienteConsola=crearCliente(8080,"127.0.0.1");
	conectarConServidor(clienteConsola);
	while (1) {
		char mensaje[SOMAXCONN];
		scanf("%s", mensaje);

		send((clienteConsola.socketCliente), mensaje, strlen(mensaje), 0);
	}
	fclose(clienteConsola.socketCliente);


	return 0;
}




