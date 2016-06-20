/*
 * configuracionesNucleo.h
 *
 *  Created on: 18/5/2016
 *      Author: utnso
 */

#ifndef CONFIGURACIONESNUCLEO_H_
#define CONFIGURACIONESNUCLEO_H_

t_reg_config get_config_params(void);
void * administrar_cola_IO(void* dispositivo);
void * administrar_cola_sem(void* semaforo);

#endif /* CONFIGURACIONESNUCLEO_H_ */
