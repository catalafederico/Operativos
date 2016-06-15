/*
 * umcClock.h
 *
 *  Created on: 10/6/2016
 *      Author: utnso
 */

#ifndef UMCCLOCK_H_
#define UMCCLOCK_H_
t_link_element* pasarDelUltAlPrimero(t_list* lista, t_link_element* ptr);
int listaEstaLlena(t_list lista,int cantMaxElementos);
t_link_element* avanzarPuntero(t_list lista,int cantMaxElementos,t_link_element* ptr);
t_link_element* buscarPaginaClk(t_list lista, t_link_element* ptr, int cantMaxElementos, int* pag);
t_link_element* actualizarLista(t_list lista, t_link_element* ptr,int cantMaxElementos, int* pag);
t_link_element* reemplazoMarco( t_list lista,t_link_element* ptr,int cantMaxElementos, int* pag);
t_link_element* agregarPagina(t_list lista,t_link_element* ptr,int cantMaxElementos, int* pag);
t_link_element* recorrerLstActualizandoBits(t_list lista, t_link_element* ptr, int cantMaxElementos, frame* pag);
t_link_element* recorrerLstYActualizarClockMod(t_list lista, t_link_element* ptr, int cantMaxElementos, frame* pag);
t_link_element* UmcClock(t_list lista, t_link_element* ptr, int cantMaxElementos, frame* pag,clock);
#endif /* UMCCLOCK_H_ */
