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
#include <arpa/inet.h>
#include <sys/socket.h>

int main(void) {
	struct sockaddr_in direccionServidor;
		direccionServidor.sin_family = AF_INET;
		direccionServidor.sin_addr.s_addr = INADDR_ANY;
		direccionServidor.sin_port = htons(8080);

		int servidor = socket(AF_INET, SOCK_STREAM, 0);

		int activado = 1;
		setsockopt(servidor, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));

		if (bind(servidor, (void*) &direccionServidor, sizeof(direccionServidor)) != 0) {
			perror("Falló el bind");
			return 1;
		}

		printf("Estoy escuchando\n");
		listen(servidor, 100);

		struct sockaddr_in direccionCliente;
		unsigned int tamanioDireccion;
		int cliente = accept(servidor, (void*) &direccionCliente, &tamanioDireccion);

		printf("Recibí una conexión en %d!!\n", cliente);
		send(cliente, "Hola!", 6, 0);

}
