/*
 * socketServer.h
 *
 *  Created on: 25/4/2016
 *      Author: utnso
 */

#ifndef SRC_SOCKETSERVER_H_
#define SRC_SOCKETSERVER_H_

struct server;
struct server crearServer();
void ponerServerEscucha(struct server preocesosServer);
void enviarMensajeACliente(char* mensaje, int socket);
void ponerServerEscuchaSelect(struct server procesosServer);
void atenderConexionNuevaSelect(struct server procesosServer, fd_set* descriptor_maestro, int* maxfichero);

#endif /* SRC_SOCKETSERVER_H_ */
