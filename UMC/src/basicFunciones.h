/*
 * basicFunciones.h
 *
 *  Created on: 21/4/2016
 *      Author: utnso
 */

#ifndef SRC_BASICFUNCIONES_H_
#define SRC_BASICFUNCIONES_H_

void crearSocket(int* socketacrear);
void abrirPuerto(int socket, struct sockaddr_in* adress);
void escucharConexiones(int socket, int colamax);
void aceptarConexion(int* socketnuevo, int socketescuchador,struct sockaddr_in* other_adress);
void enviarMensaje(int socketDestino, char* mensaje);

#endif /* SRC_BASICFUNCIONES_H_ */
