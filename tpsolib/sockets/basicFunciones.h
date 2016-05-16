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

/**
* @NAME: recibirMensaje_tamanio
* @DESC: Lee el mensaje enviado por el socket socketCliente,
* Recibe por parametro el socket (socketCliente) que envia el mensaje y
* la longitud maxima del mensaje a recibir (long_mje)
* Retorna el mensaje recibido y la longitud del mensaje en long_mje
*/
char* recibirMensaje_tamanio(int socketCliente, int * long_mje);


void* recibirStream(int socketDondeRecibe, int tamanioEstructuraARecibir);
void enviarStream(int socketDestino,int header, int tamanioMensaje, void* mensaje);

/**
* @NAME: leerHeader
* @DESC: Lee el header del mensaje, un int
* Acordarse: HACER FREE DEL INT RECIBIDO
*/

int* leerHeader(int socketARecibir());

#endif /* SRC_BASICFUNCIONES_H_ */
