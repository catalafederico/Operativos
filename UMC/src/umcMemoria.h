/*
 * umcMemoria.h
 *
 *  Created on: 9/5/2016
 *      Author: utnso
 */

#ifndef SRC_UMCMEMORIA_H_
#define SRC_UMCMEMORIA_H_
typedef struct{
	int idProg;
	int pag;
	int marco;
}tlb;
void* inicializarMemoria(t_reg_config* configuracionUMC);
int alocarPrograma(int paginasRequeridas, int id_proceso);
int desalojarPrograma(int id);
void* obtenerBytesMemoria(int pagina,int offset,int tamanio);
void almacenarBytes(int pagina, int offset, int tamanio, void* buffer);
void cambiarProceso(int idProceso);
int tablaEstaLlena(tlb tablaPag[],int cantEntradas);
void correrUnoAbajo(tlb tablaPag[],int pos);
void actualizarTablaPqEncontre(tlb tablaPag[],int i);
void actualizarTablaPqElimineUlt(tlb tablaPag[],int cantEntradas,int* pagina);
void actualizarPqNoEncontreYTablaNoLlena(tlb tablaPag[],int* pagina);
void actualizarTablaPqNoEncontre(tlb tablaPag[],int cantEntradas,int* pagina);
int buscarPagina(tlb tablaPag[],int cantEntradas,int* pagina);

#endif /* SRC_UMCMEMORIA_H_ */
