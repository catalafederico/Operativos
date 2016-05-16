/*
 * umcCliente.c
 *
 *  Created on: 13/5/2016
 *      Author: utnso
 */

#include <sockets/basicFunciones.h>
#include "estructurasUMC.h"
#define creacionPrograma 60
#define finalizacionPrograma 61

void notificarASwapPrograma(id_programa* programaCreador, int socketSwap){
	enviarStream(socketSwap,creacionPrograma,sizeof(id_programa),programaCreador);
}

void notificarASwapFinPrograma(int id, int socketSwap){
	enviarStream(socketSwap,finalizacionPrograma,sizeof(id),id);
}
