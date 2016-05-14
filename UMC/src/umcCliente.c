/*
 * umcCliente.c
 *
 *  Created on: 13/5/2016
 *      Author: utnso
 */

#include <sockets/basicFunciones.h>
#define creacionPrograma 60

typedef struct {
	int identificador;
	int paginas_requeridas;
} __attribute__((packed))
id_programa;

void notificarASwapPrograma(id_programa* programaCreador, int socketSwap){
	enviarStream(socketSwap,creacionPrograma,sizeof(id_programa),programaCreador);
}
