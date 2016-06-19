/*
 * umcMemoria.h
 *
 *  Created on: 9/5/2016
 *      Author: utnso
 */

#ifndef SRC_UMCMEMORIA_H_
#define SRC_UMCMEMORIA_H_
void* inicializarMemoria(t_reg_config* configuracionUMC);
int alocarPrograma(int paginasRequeridas, int id_proceso, t_dictionary* codigoPrograma);
int desalojarPrograma(int id);
void* obtenerBytesMemoria(int pagina,int offset,int tamanio);
void almacenarBytes(int pagina, int offset, int tamanio, void* buffer);
void cambiarProceso(int idProceso);

#endif /* SRC_UMCMEMORIA_H_ */
