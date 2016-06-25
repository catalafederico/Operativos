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
#include <pthread.h>
#include <commons/log.h>
#include "funcionesparsernuevas.h"
#include <sockets/socketCliente.h>
#include <parser/parser.h>
#include <sockets/header.h>
#include "estructurasCPU.h"
#include <signal.h>
#include "archivoConf.h"
#define SERVERUMC 9999 //puerto de la umc
#define SERVERNUCLEO 5001 // puerto del nucleo
#define OK 6
#define TAMANIOPAGINA 666
#define SOLICITAR 52

void conectarseConUMC(struct cliente clienteCpuUmc);
void conectarseConNucleo(struct cliente clienteCpuNucleo);
void procesarInstruccion(char* instruccion);
void tratarPCB();
void recibirPCB();
void enviarPCB();
void finalizarEjecucion();
struct cliente clienteCpuUmc;
struct cliente clienteCpuNucleo;
int tamanioPaginaUMC;
pcb_t* pcb_actual;
t_log* logCpu;
int quantum;
int quantumSleep;
int finEjecucion;
extern int esFuncion;
extern int estado;
extern char* nombreSemaforoWait;
extern char* nombreDispositivo;
extern int tiempo_dispositivo;


typedef struct {
	int tamanioNombreFuncion;
	int posicionPID;
}__attribute__((packed))
funcionTemp;

AnSISOP_funciones functions = {
				.AnSISOP_definirVariable = vardef,
				.AnSISOP_obtenerPosicionVariable = getvarpos,
				.AnSISOP_dereferenciar = derf,
				.AnSISOP_asignar = asignar,
				.AnSISOP_imprimir = imprimir,
				.AnSISOP_imprimirTexto = imptxt,
				.AnSISOP_irAlLabel = goint,
				.AnSISOP_asignarValorCompartida = setglobalvar,
				.AnSISOP_obtenerValorCompartida = getglobalvar,
				.AnSISOP_entradaSalida =	ionotif,
				.AnSISOP_retornar = retornar,
				.AnSISOP_llamarConRetorno = fcall,
				.AnSISOP_finalizar = fin,
				.AnSISOP_llamarSinRetorno = fcallNR
};
AnSISOP_kernel kernel_functions = { };



int main(void) {
	t_reg_config config = get_config_params();
    finEjecucion=0;
	logCpu = log_create("cpuLog.txt", "cpu", false, LOG_LEVEL_TRACE);

	//Empieza conexion UMC
	clienteCpuUmc = crearCliente(config.puertoUMC, config.IPUMC);
	log_info(logCpu, "Conectando a UMC Puerto : %d", SERVERUMC);
	conectarseConUMC(clienteCpuUmc);
	log_info(logCpu, "Conectado a UMC socket: %d", clienteCpuUmc.socketCliente);
	//Termina conexion UMC

	//Empieza conexion Nucleo
	clienteCpuNucleo = crearCliente(config.puertoNucleo, config.IPNucleo);
	log_info(logCpu, "Conectando a Nucleo Puerto : %d", SERVERNUCLEO);
	conectarseConNucleo(clienteCpuNucleo);
	log_info(logCpu, "Conectado a Nucleo socket: %d",
			clienteCpuNucleo.socketCliente);
	//Termina conexion Nucleo

	//Le paso el socket de la umc, para no pasarlo x cada pedido
	inicialzarParser(clienteCpuUmc.socketCliente,clienteCpuNucleo.socketCliente);

	//esperar la señal para cerrar a la cpu "felizmente"
	signal(SIGINT,finalizarEjecucion);

	int seguir = 1;
	while(seguir && !finEjecucion){
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

//funcion para finalizar correctamente al cpu con un ctrl c desde su consola

void finalizarEjecucion(){
	//printf("La señal es : %d",senial) en el caso de necesitar usar el int que recibe la funcion cuando la llama signal
	finEjecucion=1;
}


void conectarseConUMC(struct cliente clienteCpuUmc) {
	int a = 1;
	while(conectarConServidor(clienteCpuUmc)==-1)
	{
		printf("CPU: No se pudo conectar con UMC reintentando de 5 segundos, intento nro: %d\n", a );
		sleep(5);
		a++;
	}
	int cpuid = CPU;
	//Empieza handshake
	if (send(clienteCpuUmc.socketCliente, &cpuid, sizeof(int), 0) == -1) {
		printf("no se ha podido conectar con UMC.\n");
		perror("no anda:\0");
		log_info(logCpu, "No se pudo Conectar a UMC Puerto : %d", SERVERUMC);
	}
	int* recibido = recibirStream(clienteCpuUmc.socketCliente, sizeof(int));
	if (*recibido == OK) {
		printf("Se ha conectado correctamente con UMC.\n");
	}
	//Solicito tamanio de paginas
	int tamanioPagina = TAMANIOPAGINA;
	if (send(clienteCpuUmc.socketCliente, &tamanioPagina, sizeof(int), 0)
			== -1) {
		printf("no se ha podido solicitar tamanio de pag a la UMC.\n");
		perror("no anda:\n");
		log_info(logCpu,
				"No se pudo conectar a UMc puerto : %d al pedir tamanio de pagina",
				SERVERUMC);
	}
	recibido = leerHeader(clienteCpuUmc.socketCliente);
	if (*recibido == TAMANIOPAGINA) {
		int* recibirTamanioDePag = recibirStream(clienteCpuUmc.socketCliente,
				sizeof(int));
		tamanioPaginaUMC = *recibirTamanioDePag;
		printf("Tamanio de pagina configurado en: %d \n", tamanioPaginaUMC);
		log_info(logCpu, "Tamanio de pagina  configurado en  : %d",
				tamanioPaginaUMC);

	}
	free(recibido);
	//Termina Handshake
}

void conectarseConNucleo(struct cliente clienteCpuNucleo) {
	int a = 1;
	while(conectarConServidor(clienteCpuNucleo)==-1)
	{
		printf("CPU: No se pudo conectar con NUCLEO reintentando de 5 segundos, intento nro: %d\n", a );
		sleep(5);
		a++;
	}
	int cpuid = CPU;
	//Empieza handshake
	if (send(clienteCpuNucleo.socketCliente, &cpuid, sizeof(int), 0) == -1) {
		printf("no se ha podido conectar con UMC.\n");
		perror("no anda:\0");
		log_info(logCpu, "No se pudo conectar a Nucleo Puerto : %d",
		SERVERNUCLEO);
	}
	int* recibido = recibirStream(clienteCpuNucleo.socketCliente, sizeof(int));
	if (*recibido == OK) {
		printf("Se ha conectado correctamente con Nucleo.\n");
	}
	//Termina Handshake
}

void procesarInstruccion(char* instruccion) {
	analizadorLinea(instruccion, &functions, &kernel_functions);
}

char* proximaInstruccion() {
	//Leo la direccion de la proxima instruccion
	direccionMemoria* direcProxIntruccion = dictionary_get(
			pcb_actual->indice_codigo, pcb_actual->PC);
	//Solicito insturccion
	int solicitar = SOLICITAR;
	enviarStream(clienteCpuUmc.socketCliente, solicitar,
			sizeof(direccionMemoria), direcProxIntruccion);
	send(clienteCpuUmc.socketCliente,pcb_actual->PID,sizeof(int),0);
	//Espero instruccion
	char* proximaInstruccion = recibirStream(clienteCpuUmc.socketCliente,
			(direcProxIntruccion->tamanio));
	if(!strcmp(proximaInstruccion,"#")){
		return NULL;
	}
	return proximaInstruccion;
}

int puedeContinuarEstado(){
	//Si no puede continuar por wait io o fin
	//devuelve false
	if((estado == finalID)
			|| (estado == waitID)
			|| (estado == ioSolID)
			|| (finEjecucion)
			|| !(quantum>0)){
		return 0;
	}
	return 1;
}

void tratarPCB() {
	int hayEspacio = 1;
	do {
		hayEspacio = 1;
		//if(hayMemoria()){
			estado = 0;
			char* proxInstruccion = proximaInstruccion();
			if(proxInstruccion == NULL){
				hayEspacio = 0;
				printf("no hay espacio %d\n",*pcb_actual->PID);
				continue;
			}
			printf("procesando instruccion %d\n",*(pcb_actual->PC));
			printf("procesando inst:%s\n",proxInstruccion);
			procesarInstruccion(proxInstruccion);
			quantum--;
			*(pcb_actual->PC) = *pcb_actual->PC + 1;
			sleep(quantumSleep);
			esFuncion = 0;
		//}
		//else
			//hayEspacio = 0;
	} while (puedeContinuarEstado() && hayEspacio);

	if (estado == finalID) {
		int FIN_Proc = 1;
		send(clienteCpuNucleo.socketCliente, &FIN_Proc, sizeof(int), 0);
		enviarPCB();
		return;
	}
	if (estado == waitID) {
		int waitSem = 7;
		int tamanio = strlen(nombreSemaforoWait)+1;
		nombreSemaforoWait = strcat(nombreSemaforoWait,"\0");
		enviarStream(clienteCpuNucleo.socketCliente,waitSem,sizeof(int),&tamanio);
		send(clienteCpuNucleo.socketCliente,nombreSemaforoWait,tamanio,0);
		enviarPCB();
		free(nombreSemaforoWait);
		return;
	}
	if (estado == ioSolID) {
		int solNUCLEOIDIO = 3;
		int tamanio = strlen(nombreDispositivo)+1;
		nombreDispositivo = strcat(nombreDispositivo,"\0");
		enviarStream(clienteCpuNucleo.socketCliente,solNUCLEOIDIO,sizeof(int),&tamanio);
		send(clienteCpuNucleo.socketCliente,nombreDispositivo,tamanio,0);
		send(clienteCpuNucleo.socketCliente,&tiempo_dispositivo,sizeof(int),0);
		enviarPCB();
		free(nombreDispositivo);
		return;
	}
	if (quantum == 0 || !hayEspacio) {
		int FIN_QUAMTUM = 2;
		send(clienteCpuNucleo.socketCliente, &FIN_QUAMTUM, sizeof(int), 0);
		enviarPCB();
		return;
	}


	if(finEjecucion){
        int FIN_CPU = 4;
		printf("se finalizo la ejecucion de esta cpu \n");
		send(clienteCpuNucleo.socketCliente, &FIN_CPU, sizeof(int), 0);
		enviarPCB();
		return;
	}
}
//
int hayMemoria(){
	if(*pcb_actual->PCI != *pcb_actual->PC)
		return 1;
	int haymemoria = 123456789;
	send(clienteCpuUmc.socketCliente,&haymemoria,sizeof(int),0);
	int* recibido = recibirStream(clienteCpuUmc.socketCliente, sizeof(int));
	if(*recibido == OK){
		return 1;
	}
	else{
		return 0;
	}
}



//Enviar y recibir pcb --------------------------------------------------------

void recibirPCB() {
	pcb_t* pcb_Recibido = malloc(sizeof(pcb_t));
	pcb_Recibido->PID = recibirStream(clienteCpuNucleo.socketCliente,
			sizeof(int));
	pcb_Recibido->PC = recibirStream(clienteCpuNucleo.socketCliente,
			sizeof(int));
	pcb_Recibido->PCI = recibirStream(clienteCpuNucleo.socketCliente,
			sizeof(int));
	pcb_Recibido->SP = recibirStream(clienteCpuNucleo.socketCliente,
			sizeof(int));
	pcb_Recibido->paginasDisponible = recibirStream(
			clienteCpuNucleo.socketCliente, sizeof(int));

	pcb_Recibido->indice_codigo = dictionary_create();
	pcb_Recibido->indice_funciones = list_create();
	pcb_Recibido->indice_stack = dictionary_create();

	int* tamanioIC = recibirStream(clienteCpuNucleo.socketCliente, sizeof(int));
	int* tamanioStack = recibirStream(clienteCpuNucleo.socketCliente,
			sizeof(int));
	int* tamanioIF = recibirStream(clienteCpuNucleo.socketCliente, sizeof(int));

	int i;

//RECIBO INDICE DE CODIGO
	for (i = 0; i < *tamanioIC && *tamanioIC != 0; i++) {
		int* nuevaPagina = malloc(sizeof(int));
		*nuevaPagina = i;
		direccionMemoria* nuevaDireccionMemoria = recibirStream(
				clienteCpuNucleo.socketCliente, sizeof(direccionMemoria));
		dictionary_put(pcb_Recibido->indice_codigo, nuevaPagina,
				nuevaDireccionMemoria);
	}
	free(tamanioIC);

//RECIBO INDICE FUNCIONES
	for (i = 0; i < *tamanioIF && *tamanioIF!=0; i++) {
		funcionTemp* funcion = recibirStream(clienteCpuNucleo.socketCliente,
				sizeof(funcionTemp));
		char* funcionNombre = recibirStream(clienteCpuNucleo.socketCliente,
				funcion->tamanioNombreFuncion);
		funcion_sisop* new_funcion = malloc(sizeof(new_funcion));
		new_funcion->funcion = funcionNombre;
		new_funcion->posicion_codigo = malloc(sizeof(int));
		*(new_funcion->posicion_codigo) = funcion->posicionPID;
		list_add_in_index(pcb_Recibido->indice_funciones, i, new_funcion);
	}
	free(tamanioIF);
	int socket = clienteCpuNucleo.socketCliente;
//RECIBO STACK
	for(i=0;i<*tamanioStack && *tamanioStack!=0;i++){
		stack* stackNuevo = malloc(sizeof(stack));
		//Argumentos
		int* tamArgs = recibirStream(socket, sizeof(int));
		if(*tamArgs==-1)
			stackNuevo->args = NULL;
		else
			stackNuevo->args = list_create();
		//Variables
		int* tamVars = recibirStream(socket, sizeof(int));
		if(*tamVars==-1)
			stackNuevo->vars = NULL;
		else
			stackNuevo->vars = list_create();
		//Retorno PID del renglon stack
		int* RetornoPID = recibirStream(socket, sizeof(int));
		if(*RetornoPID == -1)
			stackNuevo->pos_ret = NULL;
		else
			stackNuevo->pos_ret = RetornoPID;
		//Retorno
		direccionMemoria* memoriaRetorno = recibirStream(socket, sizeof(direccionMemoria));
		if(memoriaRetorno->offset == -1)
			memoriaRetorno = NULL;

		stackNuevo->memoriaRetorno = memoriaRetorno;
		int j;
		for (j = 0; j < *tamArgs; j++) {
			direccionMemoria* new_direc = recibirStream(
					clienteCpuNucleo.socketCliente, sizeof(direccionMemoria));
			list_add_in_index(stackNuevo->args, j, new_direc);
		}
		free(tamArgs);
		for (j = 0; j < *tamVars; j++) {
			direccionStack* new_direc = recibirStream(
					clienteCpuNucleo.socketCliente, sizeof(direccionStack));
			list_add_in_index(stackNuevo->vars, j, new_direc);
		}
		free(tamVars);
		int* key = malloc(sizeof(int));
		*key = i;
		dictionary_put(pcb_Recibido->indice_stack, key, stackNuevo);
	}
//recibo quantum y quantumSleep
	int * temp_quantum = recibirStream(clienteCpuNucleo.socketCliente, sizeof(int));
	int * temp_quantumSleep = recibirStream(clienteCpuNucleo.socketCliente, sizeof(int));
	quantum = *temp_quantum;
	free(temp_quantum);
	quantumSleep = *temp_quantumSleep;
	free(temp_quantumSleep);
	pcb_actual = pcb_Recibido;
}

void enviarPCB() {
//NO TESTEADO
	serializablePCB aMandaNucleo;
	aMandaNucleo.PID = *(pcb_actual->PID);
	aMandaNucleo.PC = *(pcb_actual->PC);
	aMandaNucleo.PCI = *(pcb_actual->PCI);
	aMandaNucleo.SP = *(pcb_actual->SP);
	aMandaNucleo.paginasDisponible = *(pcb_actual->paginasDisponible);
	aMandaNucleo.tamanioIndiceCodigo = dictionary_size(
			pcb_actual->indice_codigo);
	aMandaNucleo.tamanioStack = dictionary_size(pcb_actual->indice_stack);
	aMandaNucleo.tamanioIndiceDeFunciones = list_size(
			pcb_actual->indice_funciones);
	//int enviaPCB = 163;
	send(clienteCpuNucleo.socketCliente,&aMandaNucleo,sizeof(serializablePCB),0);
	//enviarStream(clienteCpuNucleo.socketCliente, enviaPCB,
			//sizeof(serializablePCB), &aMandaNucleo);
	/*
	 * Orden:
	 * 	Indice de codigo
	 * 	Indice de funciones
	 * 		[int (tamanio nombre funcion)] [int posicion] [char* funcion]
	 * 	stack
	 * 		//Tamanio Args
	 * 		//Tamanio Vars
	 * 		//PidRetorno, -1 si es el main
	 *		//direccion de memoria
	 *		//Lista Args
	 *		//Lista Vars
	 *
	 */
//Empieza serializacion de listas y diccionarios
//serializo indice de codigo y lo mando
	int tamanioIndiceCode = aMandaNucleo.tamanioIndiceCodigo;
	int i;
	for (i = 0; i < tamanioIndiceCode && tamanioIndiceCode !=0; i++) {
		direccionMemoria* aMandar = dictionary_get(pcb_actual->indice_codigo,
				&i);
		send(clienteCpuNucleo.socketCliente, aMandar, sizeof(direccionMemoria),
				0);
	}

//serializo indice de funciones y lo mando
	int tamanioIndiceFunciones = aMandaNucleo.tamanioIndiceDeFunciones;
	for (i = 0; i < tamanioIndiceFunciones && tamanioIndiceFunciones != 0; i++) {
		//SS sin serializar
		//CS con serializado
		funcion_sisop* funcionAMandarSS = list_get(pcb_actual->indice_funciones,
				i);
		funcionTemp func;
		func.tamanioNombreFuncion = strlen(funcionAMandarSS->funcion) + 1;//+1 para garegar el \0
		func.posicionPID = *(funcionAMandarSS->posicion_codigo);
		//Envio Tamanio y posicion
		send(clienteCpuNucleo.socketCliente, &func, sizeof(funcionTemp), 0);
		//Envio nombre funcion
		send(clienteCpuNucleo.socketCliente,
				strcat(funcionAMandarSS->funcion, "\0"),
				func.tamanioNombreFuncion, 0);
	}

//serializo stack y lo mando
//TESTEAR MUCHO YA Q ES UN KILOMBO
	int tamanioStack = aMandaNucleo.tamanioStack;
	for (i = 0; i < tamanioStack && tamanioStack!= 0; i++) {
		stack* stackAMandar = dictionary_get(pcb_actual->indice_stack, &i);
		//Puede ser null
		int tamanioArgs;
		if(stackAMandar->args!=NULL)
			tamanioArgs = list_size(stackAMandar->args);
		else
			tamanioArgs = -1;
		//Puede ser null
		int tamanioVars;
		if(stackAMandar->vars!=NULL)
			tamanioVars = list_size(stackAMandar->vars);
		else
			tamanioVars = -1;
		//Puede ser null
		int PIDretorno;
		if(stackAMandar->pos_ret!=NULL)
			PIDretorno = *(stackAMandar->pos_ret);
		else
			PIDretorno = -1;
		//Puede ser null
		direccionMemoria direccionRetornoFuncion;
		if(stackAMandar->memoriaRetorno!=NULL)
			direccionRetornoFuncion = 	*(stackAMandar->memoriaRetorno);
		else{
			direccionRetornoFuncion.offset = -1;
			direccionRetornoFuncion.pagina = -1;
			direccionRetornoFuncion.tamanio = -1;
		}

		send(clienteCpuNucleo.socketCliente, &tamanioArgs, sizeof(int), 0);
		send(clienteCpuNucleo.socketCliente, &tamanioVars, sizeof(int), 0);
		send(clienteCpuNucleo.socketCliente, &PIDretorno, sizeof(int), 0);
		send(clienteCpuNucleo.socketCliente, &direccionRetornoFuncion,
				sizeof(direccionMemoria), 0);
		int j;
		t_list* args = stackAMandar->args;
		for (j = 0; j < tamanioArgs; j++) {
			direccionMemoria* direccionMemoriaArg = list_get(args, j);
			send(clienteCpuNucleo.socketCliente, direccionMemoriaArg,
					sizeof(direccionMemoria), 0);
		}
		t_list* vars = stackAMandar->vars;
		for (j = 0; j < tamanioVars; j++) {
			direccionStack* direccionStackVars = list_get(vars, j);
			send(clienteCpuNucleo.socketCliente, direccionStackVars,
					sizeof(direccionStack), 0);
		}
	}

//!!!!!!!!!!!!!!!!!!!!!!!!!
//LIBERAR TODA MEMORIA PCB!
//!!!!!!!!!!!!!!!!!!!!!!!!!

	return;
}

