/*
 * umcNucleo.c
 *
 *  Created on: 13/5/2016
 *      Author: utnso
 */
#include "estructurasUMC.h"
#include <stdio.h>
#include <sockets/header.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <sockets/basicFunciones.h>
#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>
#include "archivoConf.h"
#include "umcMemoria.h"
#include "umcCliente.h"

t_list* programasEjecucion;
extern umcNucleo umcConfg;
//-------- comienzo


typedef struct{
	int idProg;
	int pag;
	int marco;
}tlb;
tlb tablaPag[10];
//hay que inicializar las paginas en -1
int tablaEstaLlena(){
	for(int i=0;i<=9;i++){
		if (tablaPag[i].pag==-1){
			return 1;
		}
	}
	return 0;
}
void correrUnoAbajo(int pos){
	tlb* aux;
	aux= tablaPag[pos-1];
				tablaPag[pos]=&aux;
				pos--;
}
void actualizarTablaPqEncontre(int i){
	tlb* ptr;
	ptr=tablaPag[i];
	while(i>=0){
		if(i==0){
			tablaPag[0]=&ptr;
		}
		else{
		correrUnoAbajo(i);
		}
	}

}

void actualizarTablaPqElimineUlt(int posEliminada){
	tlb* aux;
	while(posEliminada>=0){
			if(posEliminada==0){

			//aca la pos 0 de la pag tiene basura se podria recibir lo que se quiere agregar y asignarlo
				tablaPag[0]=&aux;
			}
			else{
			correrUnoAbajo(posEliminada);
			}

}
void actualizarPqNoEncontreYTablaNoLlena(){
	int i=0;
	while(tablaPag[i].pag != -1){
				i++;
	}
			correrUnoAbajo(i);
			//a la primer posicion le asigno lo que deberia recibir
			//tablaPag[0]=
}
	//podria recibir lo que quiero agregar
void actualizarTablaPqNoEncontre(){
	if(tablaEstaLlena()){
		eliminarUltimo();
		//le paso la posicion que elimine y podria pasar lo que quiero actualizar corro todos uno hacia abajo
		actualizarTablaPqElimineUlt(9);
	}
	else{

		actualizarPqNoEncontreYTablaNoLlena();
	}
}

int buscarPagina(int* pagina){

	for(int i=0;tablaPag[i].pag != pagina & tablaPag[i].idProg !=idProcesoActual & i<=9;i++)
	{
		if (tablaPag[i].pag == pagina & tablaPag[i].idProg ==idProcesoActual)
		{
			actualizarTablaPqEncontre(i);
			return tablaPag[i].marco;
		}
	}
	return -1;
}

//-------------------- fin

void* solicitar_Bytes(int socket){
	int* pagina = (int*) recibirStream(socket, sizeof(int));
	int* offset = (int*) recibirStream(socket, sizeof(int));
	int* tamanio = (int*) recibirStream(socket, sizeof(int));
	log_info(umcConfg.loguer, "Obtener bytes iniciado");
	void* obtenido = obtenerBytesMemoria(*pagina,*offset,*tamanio);
	log_info(umcConfg.loguer, "Obtener bytes terminado");
	free(pagina);
	free(offset);
	free(tamanio);
	return obtenido;
}

void almacenar_Byte(int socket){
	int* pagina = (int*) recibirStream(socket, sizeof(int));
	int* offset = (int*) recibirStream(socket, sizeof(int));
	int* tamanio =(int*) recibirStream(socket, sizeof(int));
	void* aAlmacenar = recibirStream(socket, *tamanio);
	log_info(umcConfg.loguer, "Almacenar byte comenzado");
	almacenarBytes(*pagina,*offset,*tamanio,aAlmacenar);
	log_info(umcConfg.loguer, "Almacenar byte terminado");
	free(pagina);
	free(offset);
	free(tamanio);
	free(aAlmacenar);
	return;
}

void* conexionCpu(int socketEscuchaCpu){
	int seguir = 1;
	while(seguir){
		int* header = leerHeader(socketEscuchaCpu);
		switch (*header) {
			case 52://SOLICITAR
				solicitar_Bytes(socketEscuchaCpu);
				break;
			case 53://ALMACENAR
				almacenar_Byte(socketEscuchaCpu);
				break;
			case 666:
				enviarStream(socketEscuchaCpu,666,sizeof(int),&umcConfg.configuracionUMC.MARCO_SIZE);
				break;
			case -1:
				printf("Perdida la conexion con cpu\n");
				close(socketEscuchaCpu);
				seguir = 0;
				break;
			default:
				break;
		}
		free(header);
	}
}



