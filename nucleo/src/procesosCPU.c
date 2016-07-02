/*
 * procesosCPU.c
 *
 *  Created on: 18/5/2016
 *      Author: Lucas Marino
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include<pthread.h>

#include <unistd.h>
#include <errno.h>
//#include <netdb.h>
//#include <sys/types.h>
#include <netinet/in.h>
//#include <sys/socket.h>
//#include <sys/wait.h>
#include <signal.h>
#include <commons/collections/list.h>
//#include <commons/config.h>
#include <commons/string.h>
#include <commons/log.h>
#include <sockets/socketCliente.h>
#include <sockets/socketServer.h>
#include <sockets/basicFunciones.h>
#include "estructurasNUCLEO.h"
#include "procesosCPU.h"
#include "semaphore.h"
#include <sockets/header.h>


// CONSTANTES -----
#define SOY_CPU 	"Te_conectaste_con_CPU____"
#define SOY_UMC 	"Te_conectaste_con_UMC____"
#define SOY_SWAP	"Te_conectaste_con_SWAP___"  // 25 de long sin \0
#define SOY_NUCLEO  "Te_conectaste_con_NUCLEO_"
#define SOY_CONSOLA	"Te_conectaste_con_CONSOLA"

#define FIN_PROC 	 1
#define FIN_QUANTUM	 2
#define SOLIC_IO 	 3
#define FIN_CPU 	 4
#define OBT_VALOR 	 5
#define GRABA_VALOR  6
#define WAIT_SEM 	 7
#define SIGNAL_SEM 	 8
#define IMPRIMIR	 9
#define IMPRIMIR_TXT 10
#define SEG_FAULT 11

// Variables compartidas ---------------------------------------------
extern t_reg_config reg_config;
//extern t_list* consolas_dispo;
//extern t_list* proc_New;
extern t_list* proc_Ready;
extern t_list* proc_Exec;
extern t_list* proc_Block;
extern t_list* proc_Reject;
extern t_list* proc_Exit;
extern t_log *logger;
t_list* temp;
extern struct server serverPaCPU;

// semaforos Compartidos
extern sem_t sem_READY_dispo;
extern sem_t sem_EXIT_dispo;
extern sem_t sem_BLOCK_dispo;
extern sem_t sem_REJECT_dispo;

//extern sem_t  sem_cpus_dispo;
//extern pthread_mutex_t sem_l_cpus_dispo;
//extern t_dictionary* dict_variables;
extern t_dictionary* dict_pid_consola;
//extern t_dictionary* dict_semaforos;

//extern pthread_mutex_t sem_l_New;
extern pthread_mutex_t sem_l_Ready;
extern pthread_mutex_t sem_l_Exec;
extern pthread_mutex_t sem_l_Block;
extern pthread_mutex_t sem_l_Exit;
extern pthread_mutex_t sem_l_Reject;
extern pthread_mutex_t sem_reg_config;
extern pthread_mutex_t sem_pid_consola;
pthread_mutex_t varsGlobals = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sem_temp = PTHREAD_MUTEX_INITIALIZER;
//extern pthread_mutex_t sem_dic_semaforos;
//extern pthread_mutex_t sem_log;


void enviarCPUConHeader(int socketCPU,int header, int tamanio, void* mensaje,int pid_local);
void enviarCPU(int socketCPU, int tamanio, void* mensaje,int pid_local);
void* recibirCpu(int socketCPU, int tamanio,int pid_local);
void perdioLaConexion();

//------------------------------------------------------------------------------------------
// ---------------------------------- atender_conexion_CPU  --------------------------------
// Funcion que correra en un unico thread encargado de aceptar conexiones entrantes y generara
// un thread por cada CPU conectado y liberaran recursos automaticamente cuando dejen de ser utiles.
//------------------------------------------------------------------------------------------
void *atender_conexion_CPU(){
	log_debug(logger, "Crear socket para CPUs");
	// Crear socket para CPU  ------------------------------
	serverPaCPU = crearServer(reg_config.puerto_cpu);
	//Pongo el server a escuchar.
	ponerServerEscucha(serverPaCPU);
	log_debug(logger, "Se han empezado a escuchar cpus.");

	pthread_attr_t attr;
	pthread_t thread_cpu_con;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	temp = list_create();
	int seguir = 1;
	while(seguir){
		int* socket_nuevo = malloc(sizeof(int)); //socket donde va a estar nueva conexion
		struct sockaddr_in direccionEntrante;
		aceptarConexion(socket_nuevo, serverPaCPU.socketServer, &direccionEntrante); //No hace falta chekear si es -1, aceptarConexiones lo hace ya
		log_debug(logger, "Se ha conectado una CPU");

		int proceso = pthread_create(&thread_cpu_con , &attr ,atender_CPU, socket_nuevo);
		if( proceso < 0)
		{
			log_debug(logger, "No fue posible crear thread p/ CPU");
			exit(EXIT_FAILURE);
		}
		//No lo destruyo total todos usan el mismo atributo asi no lo creo todo el tiempo
		//pthread_attr_destroy(&attr);
		log_debug(logger, "CPU %d atendido", *socket_nuevo);


		//agrego CPU a la lista de disponibles
//		pthread_mutex_lock(&sem_l_cpus_dispo);
//			sem_post(&sem_cpus_dispo);
//		pthread_mutex_unlock(&sem_l_cpus_dispo);
	}
	return NULL;
}

void perdioLaConexion(){
 //printf("Se ha perdido la conexion con consola\n");
}
//------------------------------------------------------------------------------------------
// ---------------------------------- atender_CPU  -----------------------------------------
//Esta funcion representa un thread que trabaja con un CPU conectado por socket
//------------------------------------------------------------------------------------------
void *atender_CPU(int* socket_desc) {
	//void perdioLaConexion();
	//signal(SIGPIPE,perdioLaConexion);

	int socket_local = *socket_desc;

	//Empieza handshake
	int* recibido = recibirCpu(socket_local, sizeof(int),-1);
	if (*recibido == CPU) {
		log_debug(logger, "Se ha conectado correctamente CPU: %d",socket_local);
	}

	//Confirmo conexio a cpu
	int ok = OK;
	enviarCPU(socket_local,sizeof(int),&ok,-1);

	//Lo libero ya q era un malloc de atender_conexion_CPU
	free(socket_desc);
	int CpuActivo = 1; // 0 desactivado - 1 activado
	int cambioPcb = 0; // 0 desactivado - 1 activado
	pcb_t* pcb_elegido;
	int pid_local = 0;
	int* estado_proceso;
	t_sock_mje* socketConsola;
	int esEl_Pid(pcb_t* pcb_compara) {
			return (*pcb_compara->PID==pid_local);
	}

	while (CpuActivo) {
		//////////////////////////////////////////////
		//Le otorgo un pcb para tarabajar/////////////
		//////////////////////////////////////////////
		pid_local = -1;
		sem_wait(&sem_READY_dispo); // espero que haya un proceso en READY disponible


		pthread_mutex_lock(&sem_l_Ready);
			pcb_elegido = list_remove(proc_Ready, 0); //Agarro el pcb
			pid_local = *(pcb_elegido->PID);
			log_debug(logger, "PCB con PID %d sacado de READY", pid_local);
		pthread_mutex_unlock(&sem_l_Ready);

		pthread_mutex_lock(&sem_temp);
			list_add(temp,pcb_elegido); //Agarro el pcb
		pthread_mutex_unlock(&sem_temp);

		pthread_mutex_lock(&sem_pid_consola);
				socketConsola = dictionary_get(dict_pid_consola,&pid_local);
		pthread_mutex_unlock(&sem_pid_consola);

		cambioPcb = 0;
		if (socketConsola->proc_status!=0){ //verifico si se cerro la consola del proceso

			pthread_mutex_lock(&sem_l_Reject);
				list_add(proc_Reject, pcb_elegido);
				log_debug(logger, "PCB con PID %d pasado a REJECT xfin de consola",pid_local);
			pthread_mutex_unlock(&sem_l_Reject);
			sem_post(&sem_REJECT_dispo);
			cambioPcb = 1;
		}

		if(!cambioPcb){


			enviarPCB(pcb_elegido, socket_local, reg_config.quantum, reg_config.quantum_sleep);
			//Guardo pcb en la lista de ejecutandose
			pthread_mutex_lock(&sem_temp);
				list_remove_by_condition(temp, (void*) esEl_Pid);
			pthread_mutex_unlock(&sem_temp);

			pthread_mutex_lock(&sem_l_Exec);
				list_add(proc_Exec, pcb_elegido);
				log_debug(logger, "PCB con PID %d pasado a EXEC", pid_local);
			pthread_mutex_unlock(&sem_l_Exec);
		}
		while (!cambioPcb && CpuActivo){
			estado_proceso = recibirCpu(socket_local,sizeof(int),pid_local);
			switch (*estado_proceso) {
			case FIN_QUANTUM:
			{
				pcb_elegido = recibirPCBdeCPU(socket_local,pid_local);
				pthread_mutex_lock(&sem_l_Exec);
					list_remove_by_condition(proc_Exec, (void*) esEl_Pid);
					log_debug(logger, "PCB con PID %d sacado de EXEC x fin Quantum",pid_local);
				pthread_mutex_unlock(&sem_l_Exec);
				//
				pthread_mutex_lock(&sem_l_Ready);
					list_add(proc_Ready, pcb_elegido);
					log_debug(logger, "PCB con PID %d pasado a READY x fin Quantum",pid_local);
				pthread_mutex_unlock(&sem_l_Ready);
				sem_post(&sem_READY_dispo);
				cambioPcb = 1;//activo el cambio del pcb ya q termino el quantum
			}
				break;
			case FIN_PROC:
			{
				pcb_elegido = recibirPCBdeCPU(socket_local,pid_local);
				pthread_mutex_lock(&sem_l_Exec);
					list_remove_by_condition(proc_Exec, (void*) esEl_Pid);
					log_debug(logger, "PCB con PID %d sacado de EXEC xfin Proceso",	pid_local);
				pthread_mutex_unlock(&sem_l_Exec);
				//
				pthread_mutex_lock(&sem_l_Exit);
					list_add(proc_Exit, pcb_elegido);
					log_debug(logger, "PCB con PID %d pasado a EXIT xfin Proceso",pid_local);
				pthread_mutex_unlock(&sem_l_Exit);
				cambioPcb = 1;//activo el cambio del pcb ya q termino el proceso
				pthread_mutex_lock(&sem_pid_consola);
				socketConsola = dictionary_get(dict_pid_consola,&pid_local);
				pthread_mutex_unlock(&sem_pid_consola);
				free(socketConsola->mensaje);
				socketConsola->mensaje = strdup("Proceso_Finalizado_Correctamente");
				sem_post(&sem_EXIT_dispo);
			}
				break;
			case FIN_CPU:
			{
				pcb_elegido = recibirPCBdeCPU(socket_local,pid_local);
				pthread_mutex_lock(&sem_l_Exec);
					list_remove_by_condition(proc_Exec, (void*) esEl_Pid);
					log_debug(logger, "PCB con PID %d sacado de EXEC xfin CPU",	pid_local);
				pthread_mutex_unlock(&sem_l_Exec);
				//
				pthread_mutex_lock(&sem_l_Ready);
					list_add_in_index(proc_Ready, 0, pcb_elegido);
					log_debug(logger,"PCB con PID %d pasado al principio de READY xfin CPU",pid_local);
				pthread_mutex_unlock(&sem_l_Ready);
				sem_post(&sem_READY_dispo);
				CpuActivo = 0;
			}
				//LIBERAR PCB
				//BORAR CONSOLA DEL DICCIONARIO
				break;
			case SOLIC_IO:	//es la primitiva entradaSalida
			{
                ansisop_entradaSalida(socket_local, pid_local);
                cambioPcb = 1;//activo el cambio del pcb ya q se bloqueo el proceso
			}
				break;
			case OBT_VALOR:  //es la primitiva obtenerValorCompartida
				ansisop_obtenerValorCompartida (socket_local,pid_local);
				break;
			case GRABA_VALOR: //es la primitiva asignarValorCompartida
				ansisop_asignarValorCompartida (socket_local,pid_local);
				break;
			case WAIT_SEM:	 // es la primitiva wait
			{
				cambioPcb = ansisop_wait (socket_local, pid_local);
				enviarCPU(socket_local,sizeof(int),&cambioPcb,pid_local);
				// si cambioPcb es 0 significa que el wait no bloqueo y el cpu puede seguir procesando,
				// si es 1 entonces el proceso se bloqueo y el CPU debe tomar otro PCB
				break;
			}

			case SIGNAL_SEM: // es la primitiva signal
				ansisop_signal(socket_local,pid_local);
				break;
			case IMPRIMIR: // es la primitiva imprimir
			{
				int* valoraImprimir = recibirCpu(socket_local,sizeof(int),pid_local);
			/*	t_sock_mje* */ socketConsola = dictionary_get(dict_pid_consola,&pid_local);
				int envVar = 100;
				if(socketConsola->proc_status == 0){
					enviarCPUConHeader(socketConsola->socket_dest,envVar,sizeof(int),valoraImprimir,pid_local);
				}
				free(valoraImprimir);
				break;
			}
			case IMPRIMIR_TXT: // es la primitiva imprimirTexto
			{
			/*	t_sock_mje* */ socketConsola = dictionary_get(dict_pid_consola,&pid_local);
				int* tamanioAImprimir = recibirCpu(socket_local,sizeof(int),pid_local);
				void* mensaje = recibirCpu(socket_local,*tamanioAImprimir,pid_local);
				int envTexto = 101;
				if(socketConsola->proc_status == 0){
					enviarCPUConHeader(socketConsola->socket_dest,envTexto,sizeof(int),tamanioAImprimir,pid_local);
					enviarCPU(socketConsola->socket_dest,*tamanioAImprimir,mensaje,pid_local);
				}
				free(tamanioAImprimir);
				free(mensaje);
				break;
			}
			case SEG_FAULT:
			{
				pcb_elegido = recibirPCBdeCPU(socket_local,pid_local);
				pthread_mutex_lock(&sem_l_Exec);
					list_remove_by_condition(proc_Exec, (void*) esEl_Pid);
					log_debug(logger, "PCB con PID %d sacado de EXEC xfin Proceso",	pid_local);
				pthread_mutex_unlock(&sem_l_Exec);
				pthread_mutex_lock(&sem_l_Reject);
					list_add(proc_Reject, pcb_elegido);
					log_debug(logger, "PCB con PID %d pasado a EXIT xfin Proceso",pid_local);
				pthread_mutex_unlock(&sem_l_Reject);
				cambioPcb = 1;//activo el cambio del pcb ya q termino el proceso
				pthread_mutex_lock(&sem_pid_consola);
					socketConsola = dictionary_get(dict_pid_consola,&pid_local);
				pthread_mutex_unlock(&sem_pid_consola);
				free(socketConsola->mensaje);
				socketConsola->mensaje = strdup("Proceso_Produjo_SEG_FAULT");
				sem_post(&sem_REJECT_dispo);
				break;
			}
			default:
				break;
			}
			free(estado_proceso);
		}
	}
}



void enviarPCB(pcb_t* pcb,int cpu, int quantum, int quantum_sleep){
	//NO TESTEADO
	serializablePCB aMandaCpu;
	aMandaCpu.PID = *(pcb->PID);
	aMandaCpu.PC= *(pcb->PC);
	aMandaCpu.PCI = *(pcb->PCI);
	aMandaCpu.SP = *(pcb->SP);
	aMandaCpu.paginasDisponible = *(pcb->paginasDisponible);
	aMandaCpu.tamanioIndiceCodigo = dictionary_size(pcb->indice_codigo);
	aMandaCpu.tamanioStack = dictionary_size(pcb->indice_stack);
	aMandaCpu.tamanioIndiceDeFunciones = list_size(pcb->indice_funciones);
	int enviaPCB = 163;
	enviarCPUConHeader(cpu,enviaPCB,sizeof(serializablePCB),&aMandaCpu,*(pcb->PID));
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
	int tamanioIndiceCode =  aMandaCpu.tamanioIndiceCodigo;
	int i;
	for(i=0;i<tamanioIndiceCode && tamanioIndiceCode !=0;i++){
		direccionMemoria* aMandar = dictionary_get(pcb->indice_codigo,&i);
		enviarCPU(cpu,sizeof(direccionMemoria),aMandar,*(pcb->PID));
	}


	//serializo indice de funciones y lo mando
	int tamanioIndiceFunciones = aMandaCpu.tamanioIndiceDeFunciones;
	for(i=0;i<tamanioIndiceFunciones && tamanioIndiceFunciones != 0;i++){
		//SS sin serializar
		//CS con serializado
		funcion_sisop* funcionAMandarSS = list_get(pcb->indice_funciones,i);
		funcionTemp func;
		func.tamanioNombreFuncion = strlen(funcionAMandarSS->funcion)+1;//+1 para garegar el \0
		func.posicionPID = *(funcionAMandarSS->posicion_codigo);
		//Envio Tamanio y posicion
		enviarCPU(cpu,sizeof(funcionTemp),&func,*(pcb->PID));
		//Envio nombre funcion
		enviarCPU(cpu,func.tamanioNombreFuncion,strcat(funcionAMandarSS->funcion,"\0"),*(pcb->PID));
	}


	//serializo stack y lo mando
	//TESTEAR MUCHO YA Q ES UN KILOMBO
	int tamanioStack = aMandaCpu.tamanioStack;
	for(i=0;i<tamanioStack && tamanioStack!= 0;i++){
		stack* stackAMandar = dictionary_get(pcb->indice_stack,&i);
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
			PIDretorno = *(stackAMandar->pos_ret);//;
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

		enviarCPU(cpu,sizeof(int),&tamanioArgs,*(pcb->PID));
		enviarCPU(cpu,sizeof(int),&tamanioVars,*(pcb->PID));
		enviarCPU(cpu,sizeof(int),&PIDretorno,*(pcb->PID));
		enviarCPU(cpu,sizeof(direccionMemoria),&direccionRetornoFuncion,*(pcb->PID));
		int j;
		t_list* args = stackAMandar->args;
		for(j=0;j<tamanioArgs;j++){
			direccionMemoria* direccionMemoriaArg = list_get(args,j);
			enviarCPU(cpu,sizeof(direccionMemoria),direccionMemoriaArg,*(pcb->PID));
		}
		t_list* vars = stackAMandar->vars;
		for(j=0;j<tamanioVars;j++){
			direccionStack* direccionStackVars = list_get(vars,j);
			enviarCPU(cpu,sizeof(direccionStack),direccionStackVars,*(pcb->PID));
		}
	}

	enviarCPU(cpu,sizeof(int),&quantum,*(pcb->PID));
	enviarCPU(cpu,sizeof(int),&quantum_sleep,*(pcb->PID));

	//!!!!!!!!!!!!!!!!!!!!!!!!!
	//LIBERAR TODA MEMORIA PCB!
	//!!!!!!!!!!!!!!!!!!!!!!!!!

	return;
}

pcb_t* recibirPCBdeCPU(int socket,int pidLocal){
	pcb_t* pcb_Recibido = malloc(sizeof(pcb_t));
	pcb_Recibido->PID = recibirCpu(socket,sizeof(int),pidLocal);
	pidLocal = *(pcb_Recibido->PID);
	pcb_Recibido->PC = recibirCpu(socket,sizeof(int),pidLocal);
	pcb_Recibido->PCI = recibirCpu(socket,sizeof(int),pidLocal);
	pcb_Recibido->SP = recibirCpu(socket,sizeof(int),pidLocal);
	pcb_Recibido->paginasDisponible = recibirCpu(socket,sizeof(int),pidLocal);

	pcb_Recibido->indice_codigo = dictionary_create();
	pcb_Recibido->indice_funciones = list_create();
	pcb_Recibido->indice_stack = dictionary_create();

	int* tamanioIC = recibirCpu(socket, sizeof(int),pidLocal);
	int* tamanioStack = recibirCpu(socket, sizeof(int),pidLocal);
	int* tamanioIF = recibirCpu(socket, sizeof(int),pidLocal);

	int i;

	//RECIBO INDICE DE CODIGO
	for(i=0;i<*tamanioIC && *tamanioIC != 0;i++){
		int* nuevaPagina = malloc(sizeof(int));
		*nuevaPagina = i;
		direccionMemoria* nuevaDireccionMemoria = recibirCpu(socket,sizeof(direccionMemoria),pidLocal);
		dictionary_put(pcb_Recibido->indice_codigo,nuevaPagina,nuevaDireccionMemoria);
	}
	free(tamanioIC);


	//RECIBO INDICE FUNCIONES
	for(i=0;i<*tamanioIF && *tamanioIF!=0;i++){
		funcionTemp* funcion = recibirCpu(socket,sizeof(funcionTemp),pidLocal);
		char* funcionNombre = recibirCpu(socket,funcion->tamanioNombreFuncion,pidLocal);
		funcion_sisop* new_funcion = malloc(sizeof(new_funcion));
		new_funcion->funcion = funcionNombre;
		new_funcion->posicion_codigo = malloc(sizeof(int));
		*(new_funcion->posicion_codigo) = funcion->posicionPID;
		list_add_in_index(pcb_Recibido->indice_funciones,i,new_funcion);
	}
	free(tamanioIF);


	//RECIBO STACK
	for(i=0;i<*tamanioStack && *tamanioStack!=0;i++){
		stack* stackNuevo = malloc(sizeof(stack));
		//Argumentos
		int* tamArgs = recibirCpu(socket, sizeof(int),pidLocal);
		if(*tamArgs==-1)
			stackNuevo->args = NULL;
		else
			stackNuevo->args = list_create();
		//Variables
		int* tamVars = recibirCpu(socket, sizeof(int),pidLocal);
		if(*tamVars==-1)
			stackNuevo->vars = NULL;
		else
			stackNuevo->vars = list_create();
		//Retorno PID del renglon stack
		int* RetornoPID = recibirCpu(socket, sizeof(int),pidLocal);
		if(*RetornoPID == -1)
			stackNuevo->pos_ret = NULL;
		else
			stackNuevo->pos_ret = RetornoPID;
		//Retorno
		direccionMemoria* memoriaRetorno = recibirCpu(socket, sizeof(direccionMemoria),pidLocal);
		if(memoriaRetorno->offset == -1)
			memoriaRetorno = NULL;

		stackNuevo->memoriaRetorno = memoriaRetorno;
		int j;
		for(j=0;j<*tamArgs;j++){
			direccionMemoria* new_direc = recibirCpu(socket, sizeof(direccionMemoria),pidLocal);
			list_add_in_index(stackNuevo->args,j,new_direc);
		}
		free(tamArgs);
		for(j=0;j<*tamVars;j++){
			direccionStack* new_direc = recibirCpu(socket, sizeof(direccionStack),pidLocal);
			list_add_in_index(stackNuevo->vars,j,new_direc);
		}
		free(tamVars);
		int* key = malloc(sizeof(int));
		*key = i;
		dictionary_put(pcb_Recibido->indice_stack,key,stackNuevo);
	}
	//recibo quantum y quantumSleep
	 return pcb_Recibido;
}

void ansisop_entradaSalida(int socket_local, int pid_local){

	//se recibe parametros para IO
	int* long_char = recibirCpu(socket_local,sizeof(int),pid_local);
	char * dispositivo = recibirCpu(socket_local, *long_char,pid_local);
	int * unidades = recibirCpu(socket_local,sizeof(int),pid_local);

	//se recibe el PCB
	pcb_t* pcb_bloqueado = recibirPCBdeCPU(socket_local,pid_local);

	t_sock_mje* socketConsola;

	pthread_mutex_lock(&sem_pid_consola);
			socketConsola = dictionary_get(dict_pid_consola,&pid_local);
	pthread_mutex_unlock(&sem_pid_consola);

	int esEl_Pid(pcb_t* pcb_compara) {
			return (*pcb_compara->PID==pid_local);
	}

	pthread_mutex_lock(&sem_l_Exec);
		list_remove_by_condition(proc_Exec, (void*) esEl_Pid);
		log_debug(logger, "PCB con PID %d sacado de EXEC x bloqueo de IO",pid_local);
	pthread_mutex_unlock(&sem_l_Exec);

	if (socketConsola->proc_status==0){
		t_pcb_bloqueado* elem_block = malloc(sizeof(t_pcb_bloqueado));
		elem_block->pcb_bloqueado = pcb_bloqueado;
		elem_block->tipo_de_bloqueo = 1;
		elem_block->dispositivo = dispositivo;
		elem_block->unidades = * unidades;
		pthread_mutex_lock(&sem_l_Block); // se bloquea
			list_add(proc_Block, elem_block);
			log_debug(logger, "PCB con PID %d pasado a cola BLOCK",pid_local);
		pthread_mutex_unlock(&sem_l_Block);
		sem_post(&sem_BLOCK_dispo);
	}
	else{
		pthread_mutex_lock(&sem_l_Reject);
			list_add(proc_Reject, pcb_bloqueado);
			log_debug(logger, "PCB con PID %d pasado a REJECT xfin de consola",pid_local);
		pthread_mutex_unlock(&sem_l_Reject);
		sem_post(&sem_REJECT_dispo);
	}

}

void ansisop_obtenerValorCompartida(int socket_local,int pid_local){
	//se recibe parametros para obtener valor
	int* long_char = recibirCpu(socket_local,sizeof(int),pid_local);
	char* variable_comp = recibirCpu(socket_local, *long_char,pid_local);
	int * valor_comp;
	pthread_mutex_lock(&varsGlobals);
		valor_comp = dictionary_get(reg_config.dic_variables,variable_comp);
	pthread_mutex_unlock(&varsGlobals);

	enviarCPU(socket_local,sizeof(int),valor_comp,pid_local);
}

void ansisop_asignarValorCompartida(int socket_local,int pid_local){
	int* tamanioNombreGV = recibirCpu(socket_local,sizeof(int),pid_local);
	int* valorAGrabar = recibirCpu(socket_local, sizeof(int),pid_local);
	char* nombreVariable = recibirCpu(socket_local,*tamanioNombreGV,pid_local);
	pthread_mutex_lock(&varsGlobals);
		free(dictionary_remove(reg_config.dic_variables,nombreVariable));//libero xq es un int* malloc lo q hay
		dictionary_put(reg_config.dic_variables,nombreVariable, valorAGrabar);
	pthread_mutex_unlock(&varsGlobals);
	free(tamanioNombreGV);
	free(nombreVariable);
}

int ansisop_wait (int socket_local, int pid_local){
	int* tamanioNombreWS = recibirCpu(socket_local,sizeof(int),pid_local);
	char* nombreSemaforo = recibirCpu(socket_local,*tamanioNombreWS,pid_local);
	pcb_t* pcb_elegido = recibirPCBdeCPU(socket_local,pid_local);
	int retorno=0;
	t_datos_samaforos* datos_sem;
	t_sock_mje* socketConsola;

	int esEl_Pid(pcb_t* pcb_compara) {
			return (*pcb_compara->PID==pid_local);
	}

	datos_sem = dictionary_get(reg_config.dic_semaforos,nombreSemaforo);
	pthread_mutex_lock(&datos_sem->semsem);
	if (datos_sem->valor > 0 && list_is_empty(datos_sem->cola_procesos)){
		(datos_sem->valor)-- ;
		retorno=0;
	}
	else if (datos_sem->valor <= 0) { // Bloqueo el proceso

		pthread_mutex_lock(&sem_l_Exec);
			list_remove_by_condition(proc_Exec, (void*) esEl_Pid);
			log_debug(logger, "PCB con PID %d sacado de EXEC x bloqueo de Semaforo",pid_local);
		pthread_mutex_unlock(&sem_l_Exec);

		pthread_mutex_lock(&sem_pid_consola);
				socketConsola = dictionary_get(dict_pid_consola,&pid_local);
		pthread_mutex_unlock(&sem_pid_consola);

		if (socketConsola->proc_status==0){
			t_pcb_bloqueado* elem_block = malloc(sizeof(t_pcb_bloqueado));
			elem_block->pcb_bloqueado = pcb_elegido;
			elem_block->tipo_de_bloqueo = 2;
			elem_block->dispositivo = nombreSemaforo;
			elem_block->unidades = 0;
			pthread_mutex_lock(&sem_l_Block); // se bloquea
				list_add(proc_Block, elem_block);
				log_debug(logger, "PCB con PID %d pasado a cola BLOCK",pid_local);
			pthread_mutex_unlock(&sem_l_Block);
			sem_post(&sem_BLOCK_dispo);
		}
		else{
			pthread_mutex_lock(&sem_l_Reject);
				list_add(proc_Reject, pcb_elegido);
				log_debug(logger, "PCB con PID %d pasado a REJECT xfin de consola",pid_local);
			pthread_mutex_unlock(&sem_l_Reject);
			sem_post(&sem_REJECT_dispo);
		}

		retorno = 1; // retorna 1 si se bloquea el proceso para que cambie de PCB
	}
	pthread_mutex_unlock(&datos_sem->semsem);
	return retorno;
}

void ansisop_signal(int socket_local, int pid_local){
	int* tamanioNombre = recibirCpu(socket_local,sizeof(int),pid_local);
	char* nombreSemaforo = recibirCpu(socket_local,*tamanioNombre,pid_local);
	t_datos_samaforos* datos_sem;
	datos_sem = dictionary_get(reg_config.dic_semaforos,nombreSemaforo);
	pthread_mutex_lock(&datos_sem->semsem);
		if(!list_is_empty(datos_sem->cola_procesos))
			sem_post(&datos_sem->sem_valor);
		(datos_sem->valor)++;
	pthread_mutex_unlock(&datos_sem->semsem);

}


pcb_t* buscarYRemoverPCBporPID(int pidBuscado,t_list* lista){
	int tamanio = list_size(lista);
	int i =0;
	for (i=0;i<tamanio;i++){
		pcb_t* temp = list_get(lista,i);
		if(*(temp->PID) == pidBuscado){
			return list_remove(lista,i);
		}
	}
	return NULL;
}


void terminaDeFormaAbortiva(int pid_local) {
	//Ak aviso a consola
	int esEl_Pid(pcb_t* pcb_compara) {
		return (*pcb_compara->PID == pid_local);
	}
	printf("Se termino un CPU de forma abortiva\n");
	if (pid_local == -1) {

	} else {
		t_sock_mje* socketConsola;
		pthread_mutex_lock(&sem_l_Exec);
		pcb_t* pcb_elegido = list_remove_by_condition(proc_Exec,
				(void*) esEl_Pid);
		pthread_mutex_unlock(&sem_l_Exec);
		if(pcb_elegido == NULL){
			pthread_mutex_lock(&sem_temp);
				pcb_elegido = list_remove_by_condition(temp,
						(void*) esEl_Pid);
			pthread_mutex_unlock(&sem_temp);
			pthread_mutex_lock(&sem_l_Ready);
				list_add_in_index(proc_Ready, 0, pcb_elegido);
				log_debug(logger,"PCB con PID %d pasado al principio de READY xfin CPU",pid_local);
			pthread_mutex_unlock(&sem_l_Ready);
		}else{
		log_debug(logger, "PCB con PID %d sacado de EXEC xfin Proceso",
				pid_local);
		pthread_mutex_unlock(&sem_l_Exec);
		pthread_mutex_lock(&sem_l_Reject);
		list_add(proc_Reject, pcb_elegido);
		log_debug(logger, "PCB con PID %d pasado a EXIT xfin Proceso",
				pid_local);
		pthread_mutex_unlock(&sem_l_Reject);
		pthread_mutex_lock(&sem_pid_consola);
		socketConsola = dictionary_get(dict_pid_consola, &pid_local);
		pthread_mutex_unlock(&sem_pid_consola);
		free(socketConsola->mensaje);
		socketConsola->mensaje = strdup("123456");
		sem_post(&sem_REJECT_dispo);}
	}
	int a = 100;
	pthread_exit(&a);
}


void* recibirCpu(int socketCPU, int tamanio, int pid_local){
	void* obtenido = recibirStream(socketCPU, tamanio);
	if(obtenido==NULL){
		terminaDeFormaAbortiva(pid_local);
	}
	else{
		return obtenido;
	}
}

void enviarCPU(int socketCPU, int tamanio, void* mensaje,int pid_local){
	int resultado = send(socketCPU,mensaje,tamanio,0);
	if(resultado == -1){
		if(errno == EPIPE){
			terminaDeFormaAbortiva(pid_local);
		}
	}
}

void enviarCPUConHeader(int socketCPU,int header, int tamanio, void* mensaje,int pid_local){
	int resultado = enviarStream(socketCPU,header,tamanio,mensaje);
	if(resultado == -987){
			terminaDeFormaAbortiva(pid_local);
	}
}



