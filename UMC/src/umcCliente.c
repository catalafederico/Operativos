/*
 * umcCliente.c
 *
 *  Created on: 13/5/2016
 *      Author: utnso
 */

#include <sockets/basicFunciones.h>

#include "estructurasUMC.h"
#include <sockets/header.h>

extern umcNucleo umcConfg;
extern t_log* logConexiones;
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

void notificarASwapPrograma(int id,int paginas){
	newProgram newProceso ;
	newProceso.id = id;
	newProceso.pag = paginas;
	int nuevoPrograma = NUEVOPROGRAMA;
	log_trace(logConexiones,"Notificando nuevo programa swap id: %d pag: %d.",id,paginas);
	enviarStream(umcConfg.socketSwap,nuevoPrograma,sizeof(int),&nuevoPrograma);
}

void notificarASwapFinPrograma(int id){
	int finalid = FINALIZACIONPROGRAMA;
	log_trace(logConexiones,"Notificar swap fin programa id: %d.",id);
	enviarStream(umcConfg.socketSwap,finalid,sizeof(int),id);
}

void almacenarEnSwap(int id, int pagina, int offset, int tamanio, void* buffer){
	aEnviar almcenarSwap;
	almcenarSwap.id = id;
	almcenarSwap.pagina = pagina;
	almcenarSwap.offset = offset;
	almcenarSwap.tamanio = tamanio;
	int almc = ALMACENAR;
	enviarStream(umcConfg.socketSwap,almc,sizeof(aEnviar),&almcenarSwap);
	if(send(umcConfg.socketSwap,buffer,tamanio, 0)==-1){
		perror("error al enviar");
	};
	log_trace(logConexiones,"Notificar almacen en swap.");
}

void* solicitarEnSwap(int id, int pagina, int offset, int tamanio){
	aEnviar	solicitarSwap;
	solicitarSwap.id = id;
	solicitarSwap.pagina = pagina;
	solicitarSwap.offset = offset;
	int solc = SOLICITAR;
	log_trace(logConexiones,"Solicitar en swap.");
	enviarStream(umcConfg.socketSwap,solc,sizeof(aEnviar),&solicitarSwap);
	void* recibido = recibirStream(umcConfg.socketSwap,tamanio);
	return recibido;
}
