//------------------------------------------------------------------------------------------
// ---------------------------------- get_config_params ------------------------------------
// funcion que retorna una estructura con los datos del archivo de Configuracion de Nucleo
//------------------------------------------------------------------------------------------
#include <commons/config.h>
#include <commons/log.h>
#include "estructurasNUCLEO.h"

extern t_log *logger;

t_reg_config get_config_params(void){
//	t_log* logger = log_create("nucleo.log", "NUCLEO", 1, LOG_LEVEL_TRACE);
	t_config * archivo_config = NULL;
	char * archivo_config_nombre = "archivo_configuracion.cfg";
	t_reg_config reg_config;

	archivo_config = config_create(archivo_config_nombre);

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

	// 5 get IO_ID
	if (config_has_property(archivo_config,"IO_ID")){
		reg_config.io_id = config_get_array_value(archivo_config,"IO_ID");
	}
	else{
		log_debug(logger, "No se encontro IO_ID");
	}

	// 6 get IO_SLEEP
	if (config_has_property(archivo_config,"IO_SLEEP")){
		reg_config.io_sleep = (int *) config_get_array_value(archivo_config,"IO_SLEEP");
	}
	else{
		log_debug(logger, "No se encontro IO_SLEEP");
	}

	// 7 get SEM_ID
	if (config_has_property(archivo_config,"SEM_ID")){
		reg_config.sem_id = config_get_array_value(archivo_config,"SEM_ID");
	}
	else{
		log_debug(logger, "No se encontro SEM_ID");
	}

	// 8 get SEM_INIT
	if (config_has_property(archivo_config,"SEM_INIT")){
		reg_config.sem_init = (int *) config_get_array_value(archivo_config,"SEM_INIT");
	}
	else{
		log_debug(logger, "No se encontro SEM_INIT");
	}

	// 9 get SHARED_VARS
	if (config_has_property(archivo_config,"SHARED_VARS")){
		reg_config.shared_vars = config_get_array_value(archivo_config,"SHARED_VARS");
	}
	else{
		log_debug(logger, "No se encontro SHARED_VARS");
	}

	// 10 get STACK_SIZE
	if (config_has_property(archivo_config,"STACK_SIZE")){
		reg_config.shared_vars = config_get_int_value(archivo_config,"STACK_SIZE");
	}
	else{
		log_debug(logger, "No se encontro STACK_SIZE");
	}

	// 11 get IP_UMC
	if (config_has_property(archivo_config,"IP_UMC")){
		reg_config.shared_vars = config_get_string_value(archivo_config,"IP_UMC");
	}
	else{
		log_debug(logger, "No se encontro IP_UMC");
	}

	// 12 get PUERTO_UMC
	if (config_has_property(archivo_config,"PUERTO_UMC")){
		reg_config.shared_vars = config_get_int_value(archivo_config,"PUERTO_UMC");
	}
	else{
		log_debug(logger, "No se encontro PUERTO_UMC");
	}

	config_destroy(archivo_config);
//	log_destroy(logger);
	return reg_config;
}
