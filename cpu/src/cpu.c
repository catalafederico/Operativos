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
#include <commons/log.h>
#include "funcionesparsernuevas.h"
#include <sockets/socketCliente.h>
#include <parser/parser.h>
#include <sockets/header.h>
#include "estructurasCPU.h"
#define  SERVERUMC 9999 //puerto de la umc
#define  SERVERNUCLEO 5001 // puerto del nucleo

void conectarseConUMC(struct cliente clienteCpuUmc);
void conectarseConNucleo(struct cliente clienteCpuNucleo);
void procesarInstruccion(char* instruccion);
void tratarPCB();
void recibirPCB();
void enviarPCB();
struct cliente clienteCpuUmc;
struct cliente clienteCpuNucleo;
int tamanioPaginaUMC;
pcb_t* pcb_actual;
t_log* logCpu;
int quantum;
int quantumSleep;
extern t_list* lista_argumentos_temporales;
extern int esFuncion;

typedef struct {
	int tamanioNombreFuncion;
	int posicionPID;
}__attribute__((packed))
funcionTemp;

extern int finPrograma;

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

	logCpu = log_create("cpuLog.txt", "cpu", false, LOG_LEVEL_TRACE);

	//Empieza conexion UMC
	clienteCpuUmc = crearCliente(SERVERUMC, "127.0.0.1");
	log_info(logCpu, "Conectando a UMC Puerto : %d", SERVERUMC);
	conectarseConUMC(clienteCpuUmc);
	log_info(logCpu, "Conectado a UMC socket: %d", clienteCpuUmc.socketCliente);
	//Termina conexion UMC

	//Empieza conexion Nucleo
	clienteCpuNucleo = crearCliente(SERVERNUCLEO, "127.0.0.1");
	log_info(logCpu, "Conectando a Nucleo Puerto : %d", SERVERNUCLEO);
	conectarseConNucleo(clienteCpuNucleo);
	log_info(logCpu, "Conectado a Nucleo socket: %d",
			clienteCpuNucleo.socketCliente);
	//Termina conexion Nucleo

	//Le paso el socket de la umc, para no pasarlo x cada pedido
	inicialzarParser(clienteCpuUmc.socketCliente,
			clienteCpuNucleo.socketCliente);
	//Empieza la escucha de nucleo
	/*int b = 5;
	 pcb_t* asd ;
	 asd->PC = &seguir;
	 asd->PID = &seguir;
	 asd->paginasDisponible = &b;
	 pcb_actual = asd;*/
	//procesarInstruccion("variables a,b,c,d\n");

	int seguir = 1;
	while(seguir){
	 int* header = leerHeader(clienteCpuNucleo.socketCliente,"127.0.0.1");
	 switch (*header) {
	 case 163://Recibir PCB
		 finPrograma = 0;
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
	//Me fijo si es una funcion, como todas las funciones tienen retorno entonces todas van
	// a tener ->, por lo tanto me fijo si la linea tiene ese char*, si lo tiene
	//es funcion
	if(strstr(proximaInstruccion,"<-")!=NULL){
		esFuncion = 1;
	}
	return proximaInstruccion;
}

void tratarPCB() {
	do {
		char* proxInstruccion = proximaInstruccion();
		procesarInstruccion(proxInstruccion);
		quantum--;
		*(pcb_actual->PC) = *pcb_actual->PC + 1;
		sleep(/*quantumSleep*/3);//Cambio para testear
		list_clean(lista_argumentos_temporales);
		esFuncion = 0;
	} while (/*quantum > 0 && !finPrograma*/1);

	if (finPrograma == 1) {
		int FIN_Proc = 1;
		send(clienteCpuNucleo.socketCliente, &FIN_Proc, sizeof(int), 0);
		enviarPCB();
		return;
	}
	if (quantum == 0) {
		int FIN_QUAMTUM = 2;
		send(clienteCpuNucleo.socketCliente, &FIN_QUAMTUM, sizeof(int), 0);
		enviarPCB();
		return;
	}
}

//Enviar y recibir pcb --------------------------------------------------------

void recibirPCB() {
	pcb_t* pcb_Recibido = malloc(sizeof(pcb_t));
	pcb_Recibido->PID = recibirStream(clienteCpuNucleo.socketCliente,
			sizeof(int));
	pcb_Recibido->PC = recibirStream(clienteCpuNucleo.socketCliente,
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

//RECIBO STACK
	for (i = 0; i < *tamanioStack && *tamanioStack!=0; i++) {
		stack* stackNuevo = malloc(sizeof(stack));
		stackNuevo->args = list_create();
		stackNuevo->vars = list_create();
		int* tamArgs = recibirStream(clienteCpuNucleo.socketCliente,
				sizeof(int));
		int* tamVars = recibirStream(clienteCpuNucleo.socketCliente,
				sizeof(int));
		int* RetornoPID = recibirStream(clienteCpuNucleo.socketCliente,
				sizeof(int));
		direccionMemoria* memoriaRetorno = recibirStream(
				clienteCpuNucleo.socketCliente, sizeof(direccionMemoria));
		int j;
		for (j = 0; j < *tamArgs; j++) {
			direccionMemoria* new_direc = recibirStream(
					clienteCpuNucleo.socketCliente, sizeof(direccionMemoria));
			list_add_in_index(stackNuevo->args, j, new_direc);
		}

		for (j = 0; j < *tamVars; j++) {
			direccionStack* new_direc = recibirStream(
					clienteCpuNucleo.socketCliente, sizeof(direccionStack));
			list_add_in_index(stackNuevo->args, j, new_direc);
		}
		int* key = malloc(sizeof(int));
		*key = j;
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
	aMandaNucleo.SP = *(pcb_actual->SP);
	aMandaNucleo.paginasDisponible = *(pcb_actual->paginasDisponible);
	aMandaNucleo.tamanioIndiceCodigo = dictionary_size(
			pcb_actual->indice_codigo);
	aMandaNucleo.tamanioStack = dictionary_size(pcb_actual->indice_stack);
	aMandaNucleo.tamanioIndiceDeFunciones = list_size(
			pcb_actual->indice_funciones);
	int enviaPCB = 163;
	enviarStream(clienteCpuNucleo.socketCliente, enviaPCB,
			sizeof(serializablePCB), &aMandaNucleo);
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
		int tamanioArgs = list_size(stackAMandar->args);
		int tamanioVars = list_size(stackAMandar->vars);
		int PIDretorno = *(stackAMandar->pos_ret);
		direccionMemoria direccionRetornoFuncion =
				*(stackAMandar->memoriaRetorno);
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

