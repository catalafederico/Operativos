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
#include <sys/inotify.h>

extern t_log *logger;
extern t_reg_config reg_config;
extern pthread_mutex_t sem_reg_config;
extern pthread_mutex_t sem_l_Ready;
extern pthread_mutex_t sem_l_Reject;
extern pthread_mutex_t sem_pid_consola;
extern t_list* proc_Reject;
extern t_list* proc_Ready;
extern sem_t sem_READY_dispo;
extern sem_t sem_REJECT_dispo;
extern t_dictionary* dict_pid_consola;

//#define EVENT_SIZE  ( sizeof (struct inotify_event) )
//#define EVENT_BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )
#define EVENT_SIZE  ( sizeof (struct inotify_event) + 24 )
#define BUF_LEN     ( 1024 * EVENT_SIZE )

t_reg_config get_config_params(void){
//	t_log* logger = log_create("nucleo.log", "NUCLEO", 1, LOG_LEVEL_TRACE);
	t_config * archivo_config = NULL;
	char * archivo_config_nombre = "../archivo_configuracion.cfg";
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
    pthread_mutex_init(&(datos_sem->semsem),NULL);
    //datos_sem->semsem = PTHREAD_MUTEX_INITIALIZER;
    datos_sem->cola_procesos=list_create();
    sem_init(&datos_sem->sem_semaforos,0,0);
    sem_init(&datos_sem->sem_valor,0,0);
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
		{
			log_debug(logger, "No fue posible crear thread Admin de semaforos: %s",semaforo);
		}
		i++;
	}

//***************************************************************************
// dispara un hilo para controlar si se modifica el archivo de configuracion
//***************************************************************************
//	char * semaforo;
//	i = 0;
	pthread_t thread_conf_file;
//	semaforo = strdup(sem_id[i]);
	if(pthread_create(&thread_conf_file, NULL , observar_config_file, NULL) < 0)
	{
		log_debug(logger, "No fue posible crear thread para observar cambios en archivo de config");
	}
		i++;

	return reg_config;
}



void * administrar_cola_IO(void* dispositivo){
	log_debug(logger, "Comenzo el administrador de cola de: %s",dispositivo);
	t_pcb_bloqueado* elem_block;
	t_datos_dicIO* datos_io;
	t_sock_mje* socketConsola;

	int tiempo = 0;
	datos_io=dictionary_get(reg_config.dic_IO,dispositivo);
	while(1){
		sem_wait(&(datos_io->sem_dispositivo)); // ver si hay que 	usar el &
		elem_block = list_remove(datos_io->cola_procesos,0);
		pthread_mutex_lock(&sem_pid_consola);
				socketConsola = dictionary_get(dict_pid_consola,elem_block->pcb_bloqueado->PID);
		pthread_mutex_unlock(&sem_pid_consola);

		if (socketConsola->proc_status==0){
			tiempo = datos_io->retardo/1000 * elem_block->unidades;//divido mil para pasarlo a segundo
			sleep(tiempo);
			pthread_mutex_lock(&sem_l_Ready);
				list_add(proc_Ready,elem_block->pcb_bloqueado);
				log_debug(logger,"PCB con PID %d pasado a READY xfin de IO",*(elem_block->pcb_bloqueado->PID));
			pthread_mutex_unlock(&sem_l_Ready);
			sem_post(&sem_READY_dispo);
		}
		else{
			pthread_mutex_lock(&sem_l_Reject);
				list_add(proc_Reject, elem_block->pcb_bloqueado);
				log_debug(logger, "PCB con PID %d pasado a REJECT xfin de consola",*(elem_block->pcb_bloqueado->PID));
			pthread_mutex_unlock(&sem_l_Reject);
			sem_post(&sem_REJECT_dispo);
		}
		free(elem_block);
	}
	return 0;
}


void * administrar_cola_sem(void* semaforo){
	log_debug(logger, "Comenzo el administrador de cola de: %s",semaforo);
	t_datos_samaforos* datos_sem;
	t_pcb_bloqueado* elem_block;
	t_sock_mje* socketConsola;
	datos_sem=dictionary_get(reg_config.dic_semaforos,semaforo);
	while(1){
		sem_wait(&datos_sem->sem_valor);//Espero un signal de algun proceso
			sem_wait(&datos_sem->sem_semaforos); //espero por un proceso de ansisop
			pthread_mutex_lock(&datos_sem->semsem);
				(datos_sem->valor)--;
				elem_block = list_remove(datos_sem->cola_procesos,0);
				//dictionary_put(reg_config.dic_semaforos,elem_block->dispositivo,datos_sem);
			pthread_mutex_unlock(&datos_sem->semsem);

			pthread_mutex_lock(&sem_pid_consola);
					socketConsola = dictionary_get(dict_pid_consola,elem_block->pcb_bloqueado->PID);
			pthread_mutex_unlock(&sem_pid_consola);

			if (socketConsola->proc_status==0){
				pthread_mutex_lock(&sem_l_Ready);
					list_add(proc_Ready,elem_block->pcb_bloqueado);
					log_debug(logger,"PCB con PID %d pasado a READY xfin de Wait de semaforo: %s",*(elem_block->pcb_bloqueado->PID),semaforo);
				pthread_mutex_unlock(&sem_l_Ready);
				sem_post(&sem_READY_dispo);
			}
			else{
				pthread_mutex_lock(&sem_l_Reject);
					list_add(proc_Reject, elem_block->pcb_bloqueado);
					log_debug(logger, "PCB con PID %d pasado a REJECT xfin de consola",*(elem_block->pcb_bloqueado->PID));
				pthread_mutex_unlock(&sem_l_Reject);
				sem_post(&sem_REJECT_dispo);
				pthread_mutex_lock(&datos_sem->semsem);
					(datos_sem->valor)++;
				pthread_mutex_unlock(&datos_sem->semsem);
				sem_post(&datos_sem->sem_valor);//habilito el semaforo denuevo como si no se hubiese hecho el wait
			}
				//free(elem_block); por ahora lo saco
	}
	return 0;
}

void * observar_config_file(){
	log_debug(logger, "Comenzo el observador de archivo de config");
	printf("Comenzo el observador de archivo de config \n");
	int quantum_mod, quantum_sleep_mod;
	int nro_vez=0;
	 int length, i = 0;
	 int fd;
	 int wd;
//	 char buffer[EVENT_BUF_LEN];
	 char buffer[BUF_LEN];

	 /*creating the INOTIFY instance*/
	 fd = inotify_init();

	 /*checking for error*/
	 if ( fd < 0 ) {
		log_debug(logger, "ERROR inotify_init");
		return 0;
	 }

	 /*adding the "archivo_configuracion.cfg" file into watch list.*/
	 wd = inotify_add_watch( fd, "../archivo_configuracion.cfg", IN_MODIFY);
//	 wd = inotify_add_watch( fd, "archivo_configuracion.cfg", IN_ALL_EVENTS );
	printf("observa eventos \n");
	 while(1){
		 length=0;
		 i = 0;

		 /*read to determine the event change happens on file. This read blocks until the change event occurs*/

		 length = read( fd, buffer, BUF_LEN );
		 nro_vez++;
			printf("hubo evento \n ");
		 /*checking for error*/
		 if ( length < 0 ) {
			log_debug(logger, "ERROR inotify_read");
			return 0;
		 }

		 /*actually read return the list of change events happens. Here, read the change event one by one and process it accordingly.*/
		 while ( i < length ) {
			 struct inotify_event *event = ( struct inotify_event * ) &buffer[ i ];
//			 if ( event->len ) {
				 if ( (event->mask & IN_MODIFY) ) {
			//		 if (nro_vez%2==0){
					  log_debug(logger, "Conf File modificado, mask: %d, cookie: %d, len: %d, name: %s", event->mask, event->cookie, event->len, event->name);
							t_config * archivo_config_aux = NULL;
							char * archivo_config_nombre = "../archivo_configuracion.cfg";
							archivo_config_aux = config_create(archivo_config_nombre);
						// 3 get QUANTUM
							if (config_has_property(archivo_config_aux,"QUANTUM")){
								quantum_mod = config_get_int_value(archivo_config_aux,"QUANTUM");
							}
							else{
								log_debug(logger, "No se encontro QUANTUM");
							}

							// 4 get QUANTUM_SLEEP
							if (config_has_property(archivo_config_aux,"QUANTUM_SLEEP")){
								quantum_sleep_mod = config_get_int_value(archivo_config_aux,"QUANTUM_SLEEP");
							}
							else{
								log_debug(logger, "No se encontro QUANTUM_SLEEP");
							}

						config_destroy(archivo_config_aux);
						pthread_mutex_lock(&sem_reg_config);
						  reg_config.quantum = quantum_mod;
						  reg_config.quantum_sleep = quantum_sleep_mod;
						pthread_mutex_unlock(&sem_reg_config);

						log_debug(logger, "Se modifico el Quantum a %d",reg_config.quantum);
						log_debug(logger, "Se modifico el Quantum_Sleep a %d",reg_config.quantum_sleep);
					//--------------------------------------------------------------------------
				  // }
				 }

//			  }
			i += EVENT_SIZE + event->len;
		 }
	 }
	 /*removing the “/tmp” directory from the watch list.*/
	  inotify_rm_watch( fd, wd );
	  /*closing the INOTIFY instance*/
	  close( fd );


	return 0;
}
