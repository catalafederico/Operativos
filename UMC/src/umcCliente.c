/*
 * umcCliente.c
 *
 *  Created on: 13/5/2016
 *      Author: utnso
 */

#include <sockets/basicFunciones.h>
#include <sockets/header.h>
#include "estructurasUMC.h"

typedef struct {
	int id;
	int pagina;
	int offset;
	int tamanio;
} __attribute__((packed))
aEnviar;


void notificarASwapPrograma(id_programa* programaCreador, int socketSwap){
	enviarStream(socketSwap,NUEVOPROGRAMA,sizeof(id_programa),programaCreador);
}

void notificarASwapFinPrograma(int id, int socketSwap){
	enviarStream(socketSwap,FINALIZACIONPROGRAMA,sizeof(id),id);
}

void almacenarEnSwap(int id, int pagina, int offset, int tamanio, void* buffer, int socketSwap){
	aEnviar almcenarSwap;
	almcenarSwap.id = id;
	almcenarSwap.pagina = pagina;
	almcenarSwap.offset = offset;
	almcenarSwap.tamanio = tamanio;

	enviarStream(socketSwap,ALMACENAR,sizeof(aEnviar),&almcenarSwap);
	send(socketSwap,buffer,tamanio, 0);
}

void* solicitarEnSwap(int id, int pagina, int offset, int tamanio, int socketSwap){
	aEnviar	solicitarSwap;
	solicitarSwap.id = id;
	solicitarSwap.pagina = pagina;
	solicitarSwap.offset = offset;

	enviarStream(socketSwap,SOLICITAR,sizeof(aEnviar),&solicitarSwap);
	void* recibido = recibirStream(socketSwap,tamanio);
	return recibido;
}
