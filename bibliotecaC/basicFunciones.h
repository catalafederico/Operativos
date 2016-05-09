/*
 * basicFunciones.h
 *
 *  Created on: 21/4/2016
 *      Author: utnso
 */

#ifndef SRC_BASICFUNCIONES_H_
#define SRC_BASICFUNCIONES_H_



typedef struct {
	long desc_mje;
	long long_mje; 		// Estructura del Header de un mensaje
} t_head_mje;

void crearSocket(int* socketacrear);
void abrirPuerto(int socket, struct sockaddr_in* adress);
void escucharConexiones(int socket, int colamax);
void aceptarConexion(int* socketnuevo, int socketescuchador,struct sockaddr_in* other_adress);
void enviarMensaje(int socketDestino, char* mensaje);
char* recibirMensaje(int socketCliente);
char* recibirMensaje_tamanio(int socketCliente, int * long_mje);

#endif /* SRC_BASICFUNCIONES_H_ */
