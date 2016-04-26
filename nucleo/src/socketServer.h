/*
 * socketServer.h
 *
 *  Created on: 25/4/2016
 *      Author: utnso
 */

#ifndef SRC_SOCKETSERVER_H_
#define SRC_SOCKETSERVER_H_
#include <commons/collections/list.h>

struct server{
	int socketServer;
	struct sockaddr_in direccion;
	t_list* listaSockets;
};

struct server crearServer();
void enviarMensajeACliente(char* mensaje, int socket);
void ponerServerEscuchaSelect(struct server procesosServer);
void atenderConexionNuevaSelect(struct server procesosServer, fd_set* descriptor_maestro, int* maxfichero);

#endif /* SRC_SOCKETSERVER_H_ */
