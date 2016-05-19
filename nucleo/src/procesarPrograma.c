/*
 * procesarPrograma.c
 *
 *  Created on: 18/5/2016
 *      Author: utnso
 */

#include "estructuras.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <parser/parser.h>
#include "funcionesparsernuevas.h"
#include <commons/collections/dictionary.h>
#include <commons/collections/list.h>

extern int tamanioPaginaUMC;


AnSISOP_funciones functions = {
	.AnSISOP_definirVariable	= vardef,
	.AnSISOP_obtenerPosicionVariable= getvarpos,
	.AnSISOP_dereferenciar	= derf,
	.AnSISOP_asignar	= asignar,
	.AnSISOP_imprimir	= imprimir,
	.AnSISOP_imprimirTexto= imptxt,
	.AnSISOP_irAlLabel = goint,
	.AnSISOP_asignarValorCompartida = setglobalvar,
	.AnSISOP_obtenerValorCompartida = getglobalvar,
	.AnSISOP_entradaSalida = ionotif,
	.AnSISOP_retornar = retornar
};
AnSISOP_kernel kernel_functions = { };


indiceCodigo* nuevoPrograma(char* instrucciones,t_list* instrucc){
	indiceCodigo* ic = malloc(sizeof(indiceCodigo));
	//Copio el char en memoria de este procedimiento
	//sino no podia modificarla
	char* tempInstruccion = strdup(instrucciones);
	char* newInst = strtok(tempInstruccion,"\n");

	inicialzarPrograma();// es instruccion en 0 (false)
	ic->inst_tamanio = dictionary_create(); //inicializo indice de codigo
	int i = 0; //numero de instrucciones
	int* esInstruccion;
	do{
		analizadorLinea(newInst,&functions,&kernel_functions);
		esInstruccion = obtenerEsVariable();
		//analiso si es intruccion
		if(*esInstruccion){

			//tamanio llama un maloc para q perdure ya q es un puntero
			int* tamanio = malloc(sizeof(int));
			*tamanio = strlen(newInst)+1;
			//tamanio llama un maloc para q perdure ya q es un puntero
			int* nroInst = malloc(sizeof(int));
			//la cantidad es el tamnio del dicc
			/*0 elementos so instruccion 0
			 *1 elemento so instruccion 1
			 */
			*nroInst = dictionary_size(ic->inst_tamanio);

			// la coloco
			dictionary_put(ic->inst_tamanio,nroInst,tamanio);

			// pongo es instruccion en falso
			*esInstruccion = 0;
			list_add(instrucc,strdup(strcat(newInst,"\0")));
			}
		newInst =strtok(NULL,"\n");
		}
	while(newInst != NULL);
	int tamanioDicc = dictionary_size(ic->inst_tamanio);
	for(i=0;i<tamanioDicc;i++){
		printf("En la instruccion: %d\n",i);
		int* temp = dictionary_get(ic->inst_tamanio,&i);
		printf("Hay un tamanio de: %d\n",*temp);
		char* tempS = list_get(instrucc,i);
		printf("%s\n",tempS);
	}
	free(tempInstruccion);
	free(newInst);
	return ic;
}

t_dictionary* paginarIC(t_dictionary* codigoSinPaginar) {
	t_dictionary* diccionariConMemoria = dictionary_create();
	//Info memoria, Inicializo
	int paginaActual = 0;
	int tamanioDisponible = tamanioPaginaUMC;
	int offsetAnterior = 0;
	//Termino Info de memoria


	int instNro;
	int instTotales = dictionary_size(codigoSinPaginar);
	for (instNro = 0; instNro < instTotales; instNro++) {
		int* nroInt = malloc(sizeof(int));
		int* tamanio = malloc(sizeof(int));
		*nroInt = instNro;
		tamanio = dictionary_get(codigoSinPaginar, &instNro);
		direccionMemoria* dirInstucActual = malloc(sizeof(direccionMemoria));
		if (!(tamanioDisponible >= *tamanio)) {
			//Hay cambio de pagina, ya q la instruccion no entra en la pagina
			//Aumento la pagina
			paginaActual++;
			//resteo tamanio maximo
			tamanioDisponible = tamanioPaginaUMC;
			//reseto offset
			offsetAnterior = 0;
		}
		dirInstucActual->pagina = paginaActual;
		dirInstucActual->offset = offsetAnterior;
		dirInstucActual->tamanio = *tamanio;
		offsetAnterior +=*tamanio;
		tamanioDisponible = tamanioDisponible - *tamanio;
		dictionary_put(diccionariConMemoria,nroInt,dirInstucActual);
	}
	int tamanioDicc = dictionary_size(diccionariConMemoria);
	int i;
	for(i=0;i<tamanioDicc;i++){
		printf("En la instruccion: %d\n",i);
		direccionMemoria* temp = dictionary_get(diccionariConMemoria,&i);
		printf("Hay un tamanio de: %d\n",temp->tamanio);
		printf("pagina:%d\n",temp->pagina);
		printf("offset:%d\n",temp->offset);
	}
	return diccionariConMemoria;
}
