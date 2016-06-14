/*
 * procesosCPU.c
 *
 *  Created on: 18/5/2016
 *      Author: Lucas Marino
 */

#include <stdio.h>
//#include <string.h>
#include <stdlib.h>
#include<pthread.h>

#include <unistd.h>
//#include <errno.h>
//#include <netdb.h>
//#include <sys/types.h>
#include <netinet/in.h>
//#include <sys/socket.h>
//#include <sys/wait.h>
//#include <signal.h>
#include <commons/collections/list.h>
//#include <commons/config.h>
//#include <commons/string.h>
#include <commons/log.h>
#include <sockets/socketCliente.h>
#include <sockets/socketServer.h>
#include <sockets/basicFunciones.h>
#include "estructurasNUCLEO.h"
#include "procesosCPU.h"
#include "semaphore.h"
#include <sockets/header.h>

//FUNCIONES

pcb_t* recibirPCBdeCPU(int socket);
int* recibirEstadoProceso(int socket_local);
void *atender_CPU(int* socket_desc);


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

// Variables compartidas ---------------------------------------------
extern t_reg_config reg_config;
extern t_list* consolas_dispo;
extern t_list* proc_New;
extern t_list* proc_Ready;
extern t_list* proc_Exec;
extern t_list* proc_Block;
extern t_list* proc_Reject;
extern t_list* proc_Exit;
extern t_log *logger;
extern struct server serverPaCPU;
// semaforos Compartidos
extern sem_t sem_READY_dispo;
//extern sem_t  sem_cpus_dispo;
//extern pthread_mutex_t sem_l_cpus_dispo;
extern t_dictionary* dict_variables;

extern pthread_mutex_t sem_l_New;
extern pthread_mutex_t sem_l_Ready;
extern pthread_mutex_t sem_l_Exec;
extern pthread_mutex_t sem_l_Block;
extern pthread_mutex_t sem_l_Exit;
extern pthread_mutex_t sem_l_Reject;
extern pthread_mutex_t sem_dic_variables;
//extern pthread_mutex_t sem_log;



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



//------------------------------------------------------------------------------------------
// ---------------------------------- atender_CPU  -----------------------------------------
//Esta funcion representa un thread que trabaja con un CPU conectado por socket
//------------------------------------------------------------------------------------------
void *atender_CPU(int* socket_desc) {
	int socket_local = *socket_desc;

	//Empieza handshake
	int* recibido = recibirStream(socket_local, sizeof(int));
	if (*recibido == CPU) {
		log_debug(logger, "Se ha conectado correctamente CPU: %d",socket_local);
	}

	//Confirmo conexio a cpu
	int ok = OK;
	if (send(socket_local, &ok, sizeof(int), 0) == -1) {
		log_debug(logger, "CPU %d se Desconecto", socket_local);
		close(*socket_desc);
	}

	//Lo libero ya q era un malloc de atender_conexion_CPU
	free(socket_desc);
	int CpuActivo = 1; // 0 desactivado - 1 activado
	int cambioPcb = 0; // 0 desactivado - 1 activado
	pcb_t* pcb_elegido;
	int pid_local = 0;
	int* estado_proceso;
	while (CpuActivo) {
		//////////////////////////////////////////////
		//Le otorgo un pcb para tarabajar/////////////
		//////////////////////////////////////////////
		sem_wait(&sem_READY_dispo); // espero que haya un proceso en READY disponible
		pthread_mutex_lock(&sem_l_Ready);
			pcb_elegido = list_remove(proc_Ready, 0); //Agarro el pcb
			pid_local = *(pcb_elegido->PID);
			log_debug(logger, "PCB con PID %d sacado de NEW", pid_local);
		pthread_mutex_unlock(&sem_l_Ready);

		enviarPCB(pcb_elegido, socket_local, reg_config.quantum, reg_config.quantum_sleep);
		//Guardo pcb en la lista de ejecutandose
		pthread_mutex_lock(&sem_l_Exec);
			list_add(proc_Exec, pcb_elegido);
			log_debug(logger, "PCB con PID %d pasado a EXEC", pid_local);
		pthread_mutex_unlock(&sem_l_Exec);

		/*pcb_elegido = recibirPCBdeCPU(socket_local);
		 estado_proceso = recibirEstadoProceso(socket_local);*/


		do {
			estado_proceso = leerHeader(socket_local);
			switch (*estado_proceso && CpuActivo) {
			case FIN_QUANTUM:
				pcb_elegido = recibirPCBdeCPU(socket_local);
				pthread_mutex_lock(&sem_l_Exec);
					list_remove_by_condition(proc_Exec,	(void *) (*pcb_elegido->PID == pid_local));
					log_debug(logger, "PCB con PID %d sacado de EXEC x fin Quantum",pid_local);
				pthread_mutex_unlock(&sem_l_Exec);
				//
				pthread_mutex_lock(&sem_l_Ready);
					list_add(proc_Ready, pcb_elegido);
					log_debug(logger, "PCB con PID %d pasado a READY x fin Quantum",pid_local);
				pthread_mutex_unlock(&sem_l_Ready);
				sem_post(&sem_READY_dispo);
				cambioPcb = 1;//activo el cambio del pcb ya q termino el quantum
				break;

//			case FIN_IO:// VER SI ESTO SE MANEJA DESDE OTRO LADO,
//				pthread_mutex_lock(&sem_l_Block); // lo quito de bloqueados
//				list_remove_by_condition(proc_Block, (void *) (*pcb_elegido->PID == pid_local) );
//				pthread_mutex_unlock(&sem_l_Block);
//
//				pthread_mutex_lock(&sem_l_Ready); // lo agrego a listos
//				list_add(proc_Ready, pcb_elegido);
//				pthread_mutex_unlock(&sem_l_Ready);
//				log_debug(logger, "El proceso %d de la Consola %d pasa a READY", *pcb_elegido->PID, *pcb_elegido->con_id);
//				break;

			case FIN_PROC:
				pcb_elegido = recibirPCBdeCPU(socket_local);
				pthread_mutex_lock(&sem_l_Exec);
					list_remove_by_condition(proc_Exec,	(void *) (*pcb_elegido->PID == pid_local));
					log_debug(logger, "PCB con PID %d sacado de EXEC xfin Proceso",	pid_local);
				pthread_mutex_unlock(&sem_l_Exec);
				//
				pthread_mutex_lock(&sem_l_Exit);
					list_add(proc_Exit, pcb_elegido);
					log_debug(logger, "PCB con PID %d pasado a EXIT xfin Proceso",pid_local);
				pthread_mutex_unlock(&sem_l_Exit);
				cambioPcb = 1;//activo el cambio del pcb ya q termino el proceso
				break;

			case FIN_CPU:
				pcb_elegido = recibirPCBdeCPU(socket_local);
				pthread_mutex_lock(&sem_l_Exec);
					list_remove_by_condition(proc_Exec,	(void *) (*pcb_elegido->PID == pid_local));
					log_debug(logger, "PCB con PID %d sacado de EXEC xfin CPU",	pid_local);
				pthread_mutex_unlock(&sem_l_Exec);
				//
				pthread_mutex_lock(&sem_l_Ready);
					list_add_in_index(proc_Ready, 0, pcb_elegido);
					log_debug(logger,"PCB con PID %d pasado al principio de READY xfin CPU",pid_local);
				pthread_mutex_unlock(&sem_l_Ready);

				sem_post(&sem_READY_dispo);
				CpuActivo = 0;
				break;

//      Las siguientes son operaciones privilegiadas
			case SOLIC_IO:	//es la primitiva entradaSalida
                ansisop_entradaSalida (socket_local, pcb_elegido, pid_local);
				//              ansisop_entradaSalida ();
				//				pthread_mutex_lock(&sem_l_Block); // se bloquea
				//				list_add(proc_Block, pcb_elegido);
				//				pthread_mutex_unlock(&sem_l_Block);
				//				log_debug(logger, "El proceso %d de la Consola %d pasa a BLOCK", *pcb_elegido->PID, *pcb_elegido->con_id);
				break;

			case OBT_VALOR:  //es la primitiva obtenerValorCompartida
				//              ansisop_obtenerValorCompartida ();
				break;

			case GRABA_VALOR: //es la primitiva asignarValorCompartida
				//              ansisop_asignarValorCompartida ();
				break;

			case WAIT_SEM:	 // es la primitiva wait
				//              ansisop_wait ();
				break;

			case SIGNAL_SEM: // es la primitiva signal
				//              ansisop_signal ();
				break;

			case IMPRIMIR: // es la primitiva imprimir
				//              ansisop_imprimir();
				break;

			case IMPRIMIR_TXT: // es la primitiva imprimirTexto
				//              ansisop_imprimirTexto ();
				break;

			default:
				break;
			}
			free(estado_proceso);
		} while (!cambioPcb);

		return NULL;
	}
}



void enviarPCB(pcb_t* pcb,int cpu, int quantum, int quantum_sleep){
	//NO TESTEADO
	serializablePCB aMandaCpu;
	aMandaCpu.PID = *(pcb->PID);
	aMandaCpu.PC= *(pcb->PC);
	aMandaCpu.SP = *(pcb->SP);
	aMandaCpu.paginasDisponible = *(pcb->paginasDisponible);
	aMandaCpu.tamanioIndiceCodigo = dictionary_size(pcb->indice_codigo);
	aMandaCpu.tamanioStack = dictionary_size(pcb->indice_stack);
	aMandaCpu.tamanioIndiceDeFunciones = list_size(pcb->indice_funciones);
	int enviaPCB = 163;
	enviarStream(cpu,enviaPCB,sizeof(serializablePCB),&aMandaCpu);
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
		send(cpu,aMandar,sizeof(direccionMemoria),0);
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
		send(cpu,&func,sizeof(funcionTemp),0);
		//Envio nombre funcion
		send(cpu,strcat(funcionAMandarSS->funcion,"\0"),func.tamanioNombreFuncion,0);
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

		send(cpu,&tamanioArgs,sizeof(int),0);
		send(cpu,&tamanioVars,sizeof(int),0);
		send(cpu,&PIDretorno,sizeof(int),0);
		send(cpu,&direccionRetornoFuncion,sizeof(direccionMemoria),0);
		int j;
		t_list* args = stackAMandar->args;
		for(j=0;j<tamanioArgs;j++){
			direccionMemoria* direccionMemoriaArg = list_get(args,j);
			send(cpu,direccionMemoriaArg,sizeof(direccionMemoria),0);
		}
		t_list* vars = stackAMandar->vars;
		for(j=0;j<tamanioVars;j++){
			direccionStack* direccionStackVars = list_get(vars,j);
			send(cpu,direccionStackVars,sizeof(direccionStack),0);
		}
	}

	send(cpu,&quantum,sizeof(int),0);
	send(cpu,&quantum_sleep,sizeof(int),0);

	//!!!!!!!!!!!!!!!!!!!!!!!!!
	//LIBERAR TODA MEMORIA PCB!
	//!!!!!!!!!!!!!!!!!!!!!!!!!

	return;
}

pcb_t* recibirPCBdeCPU(int socket){
	pcb_t* pcb_Recibido = malloc(sizeof(pcb_t));
	pcb_Recibido->PID = recibirStream(socket,sizeof(int));
	pcb_Recibido->PC = recibirStream(socket,sizeof(int));
	pcb_Recibido->SP = recibirStream(socket,sizeof(int));
	pcb_Recibido->paginasDisponible = recibirStream(socket,sizeof(int));

	pcb_Recibido->indice_codigo = dictionary_create();
	pcb_Recibido->indice_funciones = list_create();
	pcb_Recibido->indice_stack = dictionary_create();

	int* tamanioIC = recibirStream(socket, sizeof(int));
	int* tamanioStack = recibirStream(socket, sizeof(int));
	int* tamanioIF = recibirStream(socket, sizeof(int));

	int i;

	//RECIBO INDICE DE CODIGO
	for(i=0;i<*tamanioIC && *tamanioIC != 0;i++){
		int* nuevaPagina = malloc(sizeof(int));
		*nuevaPagina = i;
		direccionMemoria* nuevaDireccionMemoria = recibirStream(socket,sizeof(direccionMemoria));
		dictionary_put(pcb_Recibido->indice_codigo,nuevaPagina,nuevaDireccionMemoria);
	}
	free(tamanioIC);


	//RECIBO INDICE FUNCIONES
	for(i=0;i<*tamanioIF && *tamanioIF!=0;i++){
		funcionTemp* funcion = recibirStream(socket,sizeof(funcionTemp));
		char* funcionNombre = recibirStream(socket,funcion->tamanioNombreFuncion);
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
		for(j=0;j<*tamArgs;j++){
			direccionMemoria* new_direc = recibirStream(socket, sizeof(direccionMemoria));
			list_add_in_index(stackNuevo->args,j,new_direc);
		}
		free(tamArgs);
		for(j=0;j<*tamVars;j++){
			direccionStack* new_direc = recibirStream(socket, sizeof(direccionStack));
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

int* recibirEstadoProceso(int socket_local){
	//se libera estado al final del while, por eso lo cambie a puntero
	int* estado = recibirStream(socket_local,sizeof(int));
	return estado;
}


void ansisop_entradaSalida(int socket_local, int pid_local){
	//se recibe el PCB
	pcb_t* pcb_bloqueado = recibirPCBdeCPU(socket_local);

	//se recibe parametros para IO
	int* long_char = leerHeader(socket_local);
	char * dispositivo = recibirStream(socket_local, *long_char);
	int * unidades = recibirStream(socket_local,sizeof(int));

	t_pcb_bloqueado* elem_block = malloc(sizeof(t_pcb_bloqueado));
	elem_block->pcb_bloqueado = pcb_bloqueado;
	elem_block->tipo_de_bloqueo = 1;
	elem_block->dispositivo = dispositivo;
	elem_block->unidades = * unidades;

	pthread_mutex_lock(&sem_l_Exec);
		list_remove_by_condition(proc_Exec, (void *) (*pcb_bloqueado->PID == pid_local) );
		log_debug(logger, "PCB con PID %d sacado de EXEC x bloqueo de IO",pid_local);
	pthread_mutex_unlock(&sem_l_Exec);

	pthread_mutex_lock(&sem_l_Block); // se bloquea
		list_add(proc_Block, elem_block);
		log_debug(logger, "PCB con PID %d pasado a cola BLOCK",pid_local);
	pthread_mutex_unlock(&sem_l_Block);
}

void ansisop_obtenerValorCompartida(int socket_local){
	//se recibe parametros para obtener valor
	int* long_char = recibirStream(socket_local,sizeof(int));
	char * variable_comp = recibirStream(socket_local, *long_char);
	void * valor_comp;
	pthread_mutex_lock(&sem_dic_variables);
		valor_comp = dictionary_get(dict_variables,variable_comp);
	pthread_mutex_unlock(&sem_dic_variables);

//	enviarStream();

}
