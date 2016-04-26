/*
 * socketCliente.h
 *
 *  Created on: 25/4/2016
 *      Author: utnso
 */

#ifndef SRC_SOCKETCLIENTE_H_
#define SRC_SOCKETCLIENTE_H_

struct cliente{
	int socketCliente;
	int socketServer;
	struct sockaddr_in direccionDestino;
};

struct cliente crearCliente(int puerto,char* ip);
void conectarConDireccion(int* socketMio,struct sockaddr_in* direccionDestino);
void enviarMensajeServidor(int servidorDestino,char* mensaje);
char* esperarRespuestaServidor(int socketServidor);
char* chatConProceso(int socketProceso, char* mensaje);

#endif /* SRC_SOCKETCLIENTE_H_ */
