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
typedef newProgram solcPag;
typedef newProgram almcPag;


void notificarASwapPrograma(int id,int paginas){
	newProgram newProceso ;
	newProceso.id = id;
	newProceso.pag = paginas;
	int nuevoPrograma = NUEVOPROGRAMA;
	log_trace(logConexiones,"Notificando nuevo programa swap id: %d pag: %d.",id,paginas);
	enviarStream(umcConfg.socketSwap,nuevoPrograma,sizeof(newProgram),&newProceso);
	int* conf = leerHeader(umcConfg.socketSwap);
	if(*conf==OK){
		printf("OK\n");
	}else if(*conf==ERROR){
		printf("ERROR\n");
	}
}

void notificarASwapFinPrograma(int id){
	int finalid = FINALIZACIONPROGRAMA;
	log_trace(logConexiones,"Notificar swap fin programa id: %d.",id);
	enviarStream(umcConfg.socketSwap,finalid,sizeof(int),id);
	int* conf = leerHeader(umcConfg.socketSwap);
	if(*conf==OK){
		printf("OK\n");
	}else if(*conf==ERROR){
		printf("ERROR\n");
	}
}

void almacenarEnSwap(int id, int pagina, void* buffer){
	almcPag almcenarSwap;
	almcenarSwap.id = id;
	almcenarSwap.pag = pagina;
	int almc = ALMACENAR;
	enviarStream(umcConfg.socketSwap,almc,sizeof(almcPag),&almcenarSwap);
	if(send(umcConfg.socketSwap,buffer,umcConfg.configuracionUMC.MARCO_SIZE, 0)==-1){
		perror("error al enviar\n");
	}
	log_trace(logConexiones,"Notificar almacen en swap.");
	int* conf = leerHeader(umcConfg.socketSwap);
	if(*conf==OK){
		printf("OK\n");
	}else if(*conf==ERROR){
		printf("ERROR\n");
	}
}

void* solicitarEnSwap(int id, int pagina){
	solcPag	solicitarSwap;
	solicitarSwap.id = id;
	solicitarSwap.pag = pagina;
	int solc = SOLICITAR;
	log_trace(logConexiones,"Solicitar en swap.");
	enviarStream(umcConfg.socketSwap,solc,sizeof(solcPag),&solicitarSwap);
	void* recibido = recibirStream(umcConfg.socketSwap,umcConfg.configuracionUMC.MARCO_SIZE);
	int* conf = leerHeader(umcConfg.socketSwap);
	if(*conf==OK){
		printf("OK\n");
	}else if(*conf==ERROR){
		printf("ERROR\n");
	}
	return recibido;
}
