//------------------------------------------------------------------------------------------
// ---------------------------------- get_config_params ------------------------------------
// funcion que retorna una estructura con los datos del archivo de Configuracion de Nucleo
//------------------------------------------------------------------------------------------
#include <commons/config.h>
#include <commons/log.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "estructurasNUCLEO.h"
#include "configuracionesNucleo.h"
extern t_log *logger;
extern t_reg_config reg_config;
extern pthread_mutex_t sem_reg_config;
extern pthread_mutex_t sem_l_Ready;
extern t_list* proc_Ready;
extern sem_t sem_READY_dispo;

t_reg_config get_config_params(void){
//	t_log* logger = log_create("nucleo.log", "NUCLEO", 1, LOG_LEVEL_TRACE);
	t_config * archivo_config = NULL;
	char * archivo_config_nombre = "archivo_configuracion.cfg";
//	t_reg_config reg_config;
	char** io_id;
	char** sem_id;
	char** io_sleep;
	char** sems_init;
	char** shared_vars;

	archivo_config = config_create(archivo_config_nombre);
	reg_config.dic_IO = dictionary_create();
	reg_config.dic_semaforos = dictionary_create();
	reg_config.dic_variables = dictionary_create();

	// 1 get PUERTO_PROG          --------------
	if (config_has_property(archivo_config,"PUERTO_PROG")){
		reg_config.puerto_prog = config_get_int_value(archivo_config,"PUERTO_PROG");
		log_debug(logger, "PUERTO_PROG= %d", reg_config.puerto_prog);
	}
	else{
		log_debug(logger, "No se encontro PUERTO_PROG");
	}

	// 2 get PUERTO_CPU
	if (config_has_property(archivo_config,"PUERTO_CPU")){
		reg_config.puerto_cpu = config_get_int_value(archivo_config,"PUERTO_CPU");
		log_debug(logger, "PUERTO_CPU= %d", reg_config.puerto_cpu);
	}
	else{
		log_debug(logger, "No se encontro PUERTO_CPU");
	}

	// 3 get QUANTUM
	if (config_has_property(archivo_config,"QUANTUM")){
		reg_config.quantum = config_get_int_value(archivo_config,"QUANTUM");
	}
	else{
		log_debug(logger, "No se encontro QUANTUM");
	}

	// 4 get QUANTUM_SLEEP
	if (config_has_property(archivo_config,"QUANTUM_SLEEP")){
		reg_config.quantum_sleep = config_get_int_value(archivo_config,"QUANTUM_SLEEP");
	}
	else{
		log_debug(logger, "No se encontro QUANTUM_SLEEP");
	}

	// 5 get IO_IDS
	if (config_has_property(archivo_config,"IO_IDS")){
		io_id = config_get_array_value(archivo_config,"IO_IDS");

	}
	else{
		log_debug(logger, "No se encontro IO_IDS");
	}

	// 6 get IO_SLEEP
	if (config_has_property(archivo_config,"IO_SLEEP")){
		io_sleep =  config_get_array_value(archivo_config,"IO_SLEEP");
	}
	else{
		log_debug(logger, "No se encontro IO_SLEEP");
	}

	// 7 get SEM_IDS
	if (config_has_property(archivo_config,"SEM_IDS")){
		sem_id = config_get_array_value(archivo_config,"SEM_IDS");

	}
	else{
		log_debug(logger, "No se encontro SEM_IDS");
	}

	// 8 get SEM_INIT
	if (config_has_property(archivo_config,"SEM_INIT")){
		sems_init = config_get_array_value(archivo_config,"SEM_INIT");
	}
	else{
		log_debug(logger, "No se encontro SEM_INIT");
	}

	// 9 get SHARED_VARS
	if (config_has_property(archivo_config,"SHARED_VARS")){
		shared_vars = config_get_array_value(archivo_config,"SHARED_VARS");
	}
	else{
		log_debug(logger, "No se encontro SHARED_VARS");
	}

	// 10 get STACK_SIZE
	if (config_has_property(archivo_config,"STACK_SIZE")){
		reg_config.stack_size = config_get_int_value(archivo_config,"STACK_SIZE");
	}
	else{
		log_debug(logger, "No se encontro STACK_SIZE");
	}

	// 11 get IP_UMC
	if (config_has_property(archivo_config,"IP_UMC")){
		char* temp = config_get_string_value(archivo_config,"IP_UMC");
		reg_config.ip_umc = strdup(temp);
	}
	else{
		log_debug(logger, "No se encontro IP_UMC");
	}

	// 12 get PUERTO_UMC
	if (config_has_property(archivo_config,"PUERTO_UMC")){
		reg_config.puerto_umc = config_get_int_value(archivo_config,"PUERTO_UMC");
	}
	else{
		log_debug(logger, "No se encontro PUERTO_UMC");
	}

// cargo el diccionario con los dispositivos de I_O
  int idx =0;
  t_datos_dicIO* datos_io;

  while(!(io_id[idx]==NULL)){
    datos_io=malloc(sizeof(t_datos_dicIO));
    datos_io->retardo=atoi(io_sleep[idx]);
    sem_init(&datos_io->sem_dispositivo,0,0);
    datos_io->cola_procesos=list_create();

    dictionary_put(reg_config.dic_IO,io_id[idx],datos_io);
    idx++;
  }

// cago el diccionario de semaforos dic_semaforos sem_id, sems_init
  t_datos_samaforos* datos_sem;
  idx=0;
  while(!(sem_id[idx]==NULL)){
	datos_sem = malloc(sizeof(t_datos_samaforos));
    datos_sem->valor= atoi(sems_init[idx]);
    datos_sem->cola_procesos=list_create();
    sem_init(&datos_sem->sem_semaforos,0,0);
    dictionary_put(reg_config.dic_semaforos,sem_id[idx],datos_sem);
    idx++;
  }

  	  //cago el diccionario de semaforos dic_variables
    idx=0;
    while(!(shared_vars[idx]==NULL)){
      int* valor_variable = malloc(sizeof(int));
      *valor_variable = 0;
      dictionary_put(reg_config.dic_variables,shared_vars[idx]+sizeof(char),valor_variable);//sumo uno para sacarle !
      idx++;
    }

	log_debug(logger, "Archivo de Configuracion cargado con exito");

//***************************************************************************
// dispara un hilo para atender cada dispositivo de IO
//***************************************************************************
	char * dispositivo;
	int i = 0;
	while (io_id[i]!=NULL){
		pthread_t thread_IO_admin; //hay q crear uno por cada disp
		dispositivo = strdup(io_id[i]);
		if(pthread_create(&thread_IO_admin, NULL , administrar_cola_IO,(void*) dispositivo) < 0)
		{
			log_debug(logger, "No fue posible crear thread Admin de IO: %s",dispositivo);
		//	exit(EXIT_FAILURE);
		}
		i++;
	}

//***************************************************************************
// dispara un hilo para atender semaforo
//***************************************************************************
	char * semaforo;
	i = 0;
	while (sem_id[i]!=NULL){
		pthread_t thread_sem_admin; //hay q crear uno por cada disp
		semaforo = strdup(sem_id[i]);
		if(pthread_create(&thread_sem_admin, NULL , administrar_cola_sem, (void*) semaforo) < 0)
//			if(pthread_create(&thread_sem_admin, NULL , administrar_cola_sem,NULL) < 0)
		{
			log_debug(logger, "No fue posible crear thread Admin de semaforos: %s",semaforo);
		//	exit(EXIT_FAILURE);
		}
		i++;
	}

//	config_destroy(archivo_config);
//	log_destroy(logger);
	return reg_config;
}



void * administrar_cola_IO(void* dispositivo){
	log_debug(logger, "Comenzo el administrador de cola de: %s",dispositivo);
	t_pcb_bloqueado* elem_block;
	t_datos_dicIO* datos_io;
	int tiempo = 0;
	datos_io=dictionary_get(reg_config.dic_IO,dispositivo);
	while(1){
		sem_wait(&(datos_io->sem_dispositivo)); // ver si hay que 	usar el &
		elem_block = list_remove(datos_io->cola_procesos,0);
		tiempo = datos_io->retardo/1000 * elem_block->unidades;//divido mil para pasarlo a segundo
		sleep(tiempo);
		pthread_mutex_lock(&sem_l_Ready);
			list_add(proc_Ready,elem_block->pcb_bloqueado);
			log_debug(logger,"PCB con PID %d pasado a READY xfin de IO",elem_block->pcb_bloqueado->PID);
		pthread_mutex_unlock(&sem_l_Ready);
		sem_post(&sem_READY_dispo);
		free(elem_block);
	}
	return 0;
}


void * administrar_cola_sem(void* semaforo){
	log_debug(logger, "Comenzo el administrador de cola de: %s",semaforo);
	t_datos_samaforos* datos_sem;
	t_pcb_bloqueado* elem_block;

	while(1){
//		sleep(20);
		datos_sem=dictionary_get(reg_config.dic_semaforos,semaforo);
		sem_wait(&datos_sem->sem_semaforos); // ver si hay que usar el &
		if(list_size(datos_sem->cola_procesos)>= 1){
			if(datos_sem->valor >= 1){
				pthread_mutex_lock(&sem_reg_config);
					(datos_sem->valor)--;
					elem_block = list_remove(datos_sem->cola_procesos,0);
					dictionary_put(reg_config.dic_semaforos,elem_block->dispositivo,datos_sem);
					log_debug(logger,"PCB con PID %d pasado al principio de READY xfin de IO",elem_block->pcb_bloqueado->PID);
				pthread_mutex_unlock(&sem_reg_config);

				pthread_mutex_lock(&sem_l_Ready);
					list_add(proc_Ready,elem_block->pcb_bloqueado);
					log_debug(logger,"PCB con PID %d pasado a READY xfin de Wait de semaforo: %s",elem_block->pcb_bloqueado->PID,semaforo);
				pthread_mutex_unlock(&sem_l_Ready);

				sem_post(&sem_READY_dispo);
				free(elem_block);
			}
		}

	}
	return 0;
}

