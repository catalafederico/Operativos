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

/*
	struct sockaddr_in nucleo_addr_proc;
		nucleo_addr_proc.sin_family = AF_INET;
		nucleo_addr_proc.sin_addr.s_addr = INADDR_ANY;
		nucleo_addr_proc.sin_port = htons(8080);



		int cliente = socket(AF_INET, SOCK_STREAM, 0);
			if (connect(cliente, (void*) &nucleo_addr_proc, sizeof(nucleo_addr_proc)) != 0) {
				perror("No se pudo conectar");
				return 1;}*/
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




