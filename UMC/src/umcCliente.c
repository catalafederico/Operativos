/*
 * umcCliente.c
 *
 *  Created on: 13/5/2016
 *      Author: utnso
 */

#include <sockets/basicFunciones.h>

#include "estructurasUMC.h"
#include <sockets/header.h>
typedef struct {
	int id;
	int pagina;
	int offset;
	int tamanio;
} __attribute__((packed))
aEnviar;

typedef struct {
	int id;
	int pag;
}__attribute__((packed))
newProgram;

int* socketSwap;

void inicializarSwap(int* socket){
	socketSwap = socket;
}

void notificarASwapPrograma(int id,int paginas){
	newProgram newProceso ;
	newProceso.id = id;
	newProceso.pag = paginas;
	int nuevoPrograma = NUEVOPROGRAMA;
	enviarStream(*socketSwap,nuevoPrograma,sizeof(int),&nuevoPrograma);
}

void notificarASwapFinPrograma(int id){
	int finalid = FINALIZACIONPROGRAMA;
	enviarStream(*socketSwap,finalid,sizeof(id),id);
}

void almacenarEnSwap(int id, int pagina, int offset, int tamanio, void* buffer){
	aEnviar almcenarSwap;
	almcenarSwap.id = id;
	almcenarSwap.pagina = pagina;
	almcenarSwap.offset = offset;
	almcenarSwap.tamanio = tamanio;
	int almc = ALMACENAR;
	enviarStream(*socketSwap,almc,sizeof(aEnviar),&almcenarSwap);
	if(send(socketSwap,buffer,tamanio, 0)==-1){
		perror("error al enviar");
	};
}

void* solicitarEnSwap(int id, int pagina, int offset, int tamanio){
	aEnviar	solicitarSwap;
	solicitarSwap.id = id;
	solicitarSwap.pagina = pagina;
	solicitarSwap.offset = offset;
	int solc = SOLICITAR;
	enviarStream(*socketSwap,solc,sizeof(aEnviar),&solicitarSwap);
	void* recibido = recibirStream(socketSwap,tamanio);
	return recibido;
}
