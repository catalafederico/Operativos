/*
 ============================================================================
 Name        : cpu.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <commons/collections/list.h>
#include <sockets/header.h>
#include "funcionesparsernuevas.h"
#include <sockets/socketCliente.h>
#include <parser/parser.h>

#include "estructurasCPU.h"


AnSISOP_funciones functions = {
	.AnSISOP_definirVariable	= vardef,
	.AnSISOP_obtenerPosicionVariable= getvarpos,
	.AnSISOP_dereferenciar	= derf,
	.AnSISOP_asignar	= asignar,
	.AnSISOP_imprimir	= imprimir,
	.AnSISOP_imprimirTexto= imptxt,
	.AnSISOP_irAlLabel = goint,
	.AnSISOP_asignarValorCompartida = setglobalvar,
	.AnSISOP_obtenerValorCompartida = getglobalvar,
	.AnSISOP_entradaSalida = ionotif,
	.AnSISOP_retornar = retornar
};
AnSISOP_kernel kernel_functions = { };

struct cliente clienteCpuUmc;
struct cliente clienteCpuNucleo;
int tamanioPaginaUMC;
pcb_t* pcb_actual;
#define  SERVERUMC 9999 //puerto de la umc
#define  SERVERNUCLEO 5001 // puerto del nucleo

void conectarseConUMC(struct cliente clienteCpuUmc);
void procesarInstruccion(char* instruccion);

int main(void) {

	//Empieza conexion UMC
	clienteCpuUmc = crearCliente(SERVERUMC, "127.0.0.1");
	conectarseConUMC(clienteCpuUmc);
	//Termina conexion UMC

	//Empieza conexion Nucleo
	clienteCpuNucleo = crearCliente(SERVERNUCLEO, "127.0.0.1");
	conectarseConNucleo(clienteCpuNucleo);
	//Termina conexion Nucleo

	//Le paso el socket de la umc, para no pasarlo x cada pedido
	inicialzarParser(clienteCpuUmc.socketCliente,clienteCpuNucleo.socketCliente);

	//Empieza la escucha de nucleo
	int seguir = 1;
	while(seguir){
		int* header = leerHeader(clienteCpuNucleo.socketCliente,"127.0.0.1");
		switch (*header) {
			case 163://Recibir PCB
				recibirPCB();
				tratarPCB();
				break;
			default:
				break;
		}
		free(header);
	}
	return 0;
}

void conectarseConUMC(struct cliente clienteCpuUmc){
	conectarConServidor(clienteCpuUmc);
	int cpuid = CPU;
	//Empieza handshake
	if(send(clienteCpuUmc.socketCliente,&cpuid,sizeof(int),0)==-1){
		printf("no se ha podido conectar con UMC.\n");
		perror("no anda:\0");
	}
	int* recibido = recibirStream(clienteCpuUmc.socketCliente,sizeof(int));
	if(*recibido==OK){
		printf("Se ha conectado correctamente con UMC.\n");
	}
	//Solicito tamanio de paginas
	int tamanioPagina = TAMANIOPAGINA;
	if(send(clienteCpuUmc.socketCliente,&tamanioPagina,sizeof(int),0)==-1){
		printf("no se ha podido solicitar tamanio de pag a la UMC.\n");
		perror("no anda:\n");
	}
	recibido = leerHeader(clienteCpuUmc.socketCliente);
	if(*recibido==TAMANIOPAGINA){
		int* recibirTamanioDePag = recibirStream(clienteCpuUmc.socketCliente,sizeof(int));
		tamanioPaginaUMC = *recibirTamanioDePag;
		printf("Tamanio de pagina configurado en: %d \n", tamanioPaginaUMC);
	}
	free(recibido);
	//Termina Handshake
}

void conectarseConNucleo(struct cliente clienteCpuNucleo){
	conectarConServidor(clienteCpuNucleo);
	int cpuid = CPU;
	//Empieza handshake
	if(send(clienteCpuNucleo.socketCliente,&cpuid,sizeof(int),0)==-1){
		printf("no se ha podido conectar con UMC.\n");
		perror("no anda:\0");
	}
	int* recibido = recibirStream(clienteCpuNucleo.socketCliente,sizeof(int));
	if(*recibido==OK){
		printf("Se ha conectado correctamente con UMC.\n");
	}
	//Termina Handshake
}

void procesarInstruccion(char* instruccion){
	analizadorLinea(instruccion,&functions,&kernel_functions);
}


void recibirPCB(){
	pcb_t* pcb_Recibido = malloc(sizeof(pcb_t));
	//Recibo un void* lo casteo a un int* y obtengo valor, asi en todos
	pcb_Recibido->PID = recibirStream(clienteCpuNucleo.socketCliente,sizeof(int));
	pcb_Recibido->PC = recibirStream(clienteCpuNucleo.socketCliente,sizeof(int));
	pcb_Recibido->SP = recibirStream(clienteCpuNucleo.socketCliente,sizeof(int));
	pcb_Recibido->paginasDisponible = recibirStream(clienteCpuNucleo.socketCliente,sizeof(int));
	int* tamanioIC = recibirStream(clienteCpuNucleo.socketCliente, sizeof(int));
	pcb_Recibido->indice_codigo = dictionary_create();
	int i;
	for(i=0;i<*tamanioIC;i++){
		int* nuevaPagina = malloc(sizeof(int));
		*nuevaPagina = i;
		direccionMemoria* nuevaDireccionMemoria = recibirStream(clienteCpuNucleo.socketCliente,sizeof(direccionMemoria));
		dictionary_put(pcb_Recibido->indice_codigo,nuevaPagina,nuevaDireccionMemoria);
	}
	free(tamanioIC);
	pcb_actual = pcb_Recibido;
}

void enviarPCB(){
	serializablePCB aMandaNucleo;
	aMandaNucleo.PID = *(pcb_actual->PID);
	aMandaNucleo.PC= *(pcb_actual->PC);
	aMandaNucleo.SP = *(pcb_actual->SP);
	aMandaNucleo.paginasDisponible = *(pcb_actual->paginasDisponible);
	aMandaNucleo.tamanioIC = dictionary_size(pcb_actual->indice_codigo);
	int enviaPCB = 163;
	enviarStream(clienteCpuNucleo.socketCliente,enviaPCB,sizeof(serializablePCB),&aMandaNucleo);
	//serializo diccionario y lo mando
	int tamanioIndiceCode =  dictionary_size(pcb_actual->indice_codigo);
	int i;
	for(i=0;i<tamanioIndiceCode;i++){
		direccionMemoria* aMandar = dictionary_get(pcb_actual->indice_codigo,&i);
		send(clienteCpuNucleo.socketCliente,aMandar,sizeof(direccionMemoria),0);
	}
	return;
}


void tratarPCB(){
	//Leo la direccion de la proxima instruccion
	direccionMemoria* direcProxIntruccion = dictionary_get(pcb_actual->indice_codigo,pcb_actual->PC);
	//Aumento ip
	pcb_actual->PC = pcb_actual->PC+1;
	//Solicito insturccion
	int solicitar = SOLICITAR;
	enviarStream(clienteCpuUmc.socketCliente,&solicitar,sizeof(direccionMemoria),direcProxIntruccion);
	//Espero instruccion
	char* proximaInstruccion = recibirStream(clienteCpuUmc.socketCliente,direcProxIntruccion->tamanio);
	//Analiso Instruccion
	procesarInstruccion(proximaInstruccion);

}
