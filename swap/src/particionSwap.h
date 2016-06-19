/*
 * particionSwap.h
 *
 *  Created on: 18/6/2016
 *      Author: utnso
 */

#ifndef PARTICIONSWAP_H_
#define PARTICIONSWAP_H_

void crearArchivo();
void inicializarArchivo();
void escribirPagina(int paginaAEscribir, void* texto);
void* leerPagina(int pagina);
void compactar(void);
void moverPaginas(proceso* procesoAJuntar, int nuevoComienzo);


#endif /* PARTICIONSWAP_H_ */
