/*
 * procesarPrograma.c
 *
 *  Created on: 18/5/2016
 *      Author: utnso
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <parser/parser.h>
#include "funcionesparsernuevas.h"
#include <commons/collections/dictionary.h>
#include <commons/collections/list.h>
#include <sockets/header.h>
#include "estructurasNUCLEO.h"
#include <commons/string.h>

extern int tamanioPaginaUMC;
extern int esFuncion;
extern char* nombreFuncion;
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
	.AnSISOP_retornar = retornar,
	.AnSISOP_llamarConRetorno = fcall,
	.AnSISOP_finalizar = fin,
	.AnSISOP_llamarSinRetorno = fcallNR
};
AnSISOP_kernel kernel_functions = { };

//Se crea inidice de codigo e indice funciones

indiceCodigo* nuevoPrograma(char* instrucciones,t_list* instrucc,t_list* lista_Inst_pcb){
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
		char *aAnalizar = strdup(newInst); //tuve q poner esto asi sino me tiraba error el strtrim
		newInst =strtok(NULL,"\n");
		string_trim(&aAnalizar);
		//En eso 4 pasos verifico si es valido la instruccion
		if(!strcmp(aAnalizar,"begin"))
			continue;
		//if(!strcmp(aAnalizar,"end"))
			//break;
		if(string_is_empty(aAnalizar))
			continue;
		if(string_starts_with(aAnalizar,"#"))
			continue;
		if(string_starts_with(aAnalizar,":"))
		{
			char* temp = strdup(aAnalizar+sizeof(char));
			funcion_sisop* nuevaFc = malloc(sizeof(nuevaFc));
			nuevaFc->funcion = temp;
			nuevaFc->posicion_codigo = malloc(sizeof(int));
			*(nuevaFc->posicion_codigo) = i;
			list_add(lista_Inst_pcb,nuevaFc);
			esFuncion = 0;
			continue;
		}


		analizadorLinea(aAnalizar,&functions,&kernel_functions);
		esInstruccion = obtenerEsVariable();
		//analizo si es intruccion

		if(*esInstruccion){
			//tamanio llama un maloc para q perdure ya q es un puntero
			int* tamanio = malloc(sizeof(int));
			*tamanio = strlen(aAnalizar)+1;
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
			list_add(instrucc,strdup(strcat(aAnalizar,"\0")));
			if(esFuncion){

				char** palabrasaAnalizar = string_split(aAnalizar," ");
				funcion_sisop* nuevaFc = malloc(sizeof(nuevaFc));
				char* palabra = strdup(palabrasaAnalizar[1]);
				string_trim(&palabra);
				nuevaFc->funcion = palabra;
				nuevaFc->posicion_codigo = malloc(sizeof(int));
				*(nuevaFc->posicion_codigo) = i;
				list_add(lista_Inst_pcb,nuevaFc);
				esFuncion = 0;
				}
			i++;
			}
		free(aAnalizar);
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
	//free(newInst);
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
		while (!(tamanioDisponible >= *tamanio)) {
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

int cargarEnUMC(t_dictionary* codigoPaginado,t_list* instrucciones, int pagNecesarias,int socetUMC,int id){
	int newProgram = NUEVOPROGRAMA;
	programa_id prog;
	prog.id = id;
	prog.pagnias = pagNecesarias;
	enviarStream(socetUMC,newProgram,sizeof(programa_id),&prog);//Solicito paginas a la umc
	//Liberarlo solo el codigo4UMC!
	t_dictionary* codigo4UMC = dictionary_create();
	int mensajesAEnviar = list_size(instrucciones);
	int i;
	for(i=0;i<mensajesAEnviar;i++){
		//Agarro la direccion de la instruccion y se la envia para informarle donde almacenar la instruccion
		//No cambiar orden ya q umc recibe asi
		direccionMemoria* aAlmacenar = dictionary_get(codigoPaginado,&i);
		//puntero a la instruccion a enviar
		char* instruccionAENviar = list_get(instrucciones,i);
		if(!dictionary_has_key(codigo4UMC,&aAlmacenar->pagina)){
			void* buffer = malloc(tamanioPaginaUMC);
			memcpy(buffer,instruccionAENviar,aAlmacenar->tamanio);
			dictionary_put(codigo4UMC,&aAlmacenar->pagina,buffer);
		}
		else{
			void* buffer = dictionary_get(codigo4UMC,&aAlmacenar->pagina);
			memcpy(buffer+aAlmacenar->offset,instruccionAENviar,aAlmacenar->tamanio);
		}
	}

	int cantPagCodigo = dictionary_size(codigo4UMC);
	for(i=0;i<cantPagCodigo;i++){
		void* bufferAEnviar = dictionary_get(codigo4UMC,&i);
		enviarStream(socetUMC,i,tamanioPaginaUMC,bufferAEnviar);
	}

	int finaAlocar =-1;
	send(socetUMC,&finaAlocar,sizeof(int),0);

	int* header = recibirStream(socetUMC,sizeof(int));
	if(*header==ERROR){
		return -1;//No hay paginas disponibles
	}
	return 0;
}
