/*
 * umcConsola.c
 *
 *  Created on: 29/4/2016
 *      Author: utnso
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "estructurasUMC.h"
#include <commons/string.h>

#define ayuda "help"
#define retardo "delay"
#define dumpStrMemory "dstrmry"
#define dumpContMemory "dcontmry"
#define flushTlb "ftlb"
#define flushMry "fmry"
#define cerrar "close"

extern t_dictionary* programas_ejecucion;
extern pthread_mutex_t semaforoMemoria;
extern umcNucleo umcConfg;
extern void* memoriaPrincipal;
int vecesRepetir;
void* mostrarEstructura(char* programaAImprimir, void* pag_marco);
void* mostrarContenidoDeMemoria(char* programaAImprimir, void* pag_marco);

void consolaUMC(){
	char* comandoCompleto = malloc(50);
	char* primerPalabra;
	vecesRepetir = umcConfg.configuracionUMC.MARCO_SIZE/1;
	do{
		printf("Ingrese un comando.(help para ver comandos.) \n");
		fgets(comandoCompleto,50,stdin);
		int i;
		i = strlen(comandoCompleto)-1;
		if( comandoCompleto[ i ] == '\n')
			  comandoCompleto[i] = '\0';
		char ** todasLasPalabras = string_split(comandoCompleto, " ");
		primerPalabra = todasLasPalabras[0];
		printf("Ha ingresado:\n\t------%s-------\n",comandoCompleto);
		if(!strcmp(primerPalabra,ayuda)){
			printf("Comando ayuda activado\n");
			printf("Comando delay: %s\n", retardo);
			printf("Comando dump struct memory: %s\n", dumpStrMemory);
			printf("Comando dump contenido memory: %s\n", dumpContMemory);
			printf("Comando flush tlb: %s\n", flushTlb);
			printf("Comando flush memory: %s\n", flushMry);
			printf("Comando cerrar: %s\n", cerrar);
		}
		else if(!strcmp(primerPalabra,retardo)){
			char* delay = todasLasPalabras[1];
			int nuevoDelay = strtol(delay,NULL,10);
			umcConfg.configuracionUMC.RETARDO = nuevoDelay;
			printf("Delay nuevo seteado en: %d\n",nuevoDelay);
		}
		else if(!strcmp(primerPalabra,dumpStrMemory)){
			char* programa = todasLasPalabras[1];
			if(programa == NULL){
				printf("No ha seleccionado ningun proceso en especial se imprimira las estructuras de todos los procesos:\n");
				dictionary_iterator(programas_ejecucion,mostrarEstructura);
			}
			else{
				int programaAImprimir = strtol(programa,NULL,10);
				printf("Ha seleccionado el procesos con ID %d: \n", programaAImprimir);
				if(dictionary_has_key(programas_ejecucion,&programaAImprimir)){
					t_dictionary* pag_marco = dictionary_get(programas_ejecucion,&programaAImprimir);
					mostrarEstructura(&programaAImprimir,pag_marco);
				}else{
					printf("No hay ningun proceso con id %d\n",programaAImprimir);
				}
			}
		}
		else if(!strcmp(primerPalabra,dumpContMemory)){
			char* programa = todasLasPalabras[1];
			if(programa == NULL){
				printf("No ha seleccionado ningun proceso en especial se imprimira el contenido de todos los procesos:\n");
				dictionary_iterator(programas_ejecucion,mostrarContenidoDeMemoria);
			}
			else{
				int programaAImprimir = strtol(programa,NULL,10);
				printf("Ha seleccionado el procesos con ID %d: \n", programaAImprimir);
				if(dictionary_has_key(programas_ejecucion,&programaAImprimir)){
					t_dictionary* pag_marco = dictionary_get(programas_ejecucion,&programaAImprimir);
					//mostrarEstructura(&programaAImprimir,pag_marco);
					mostrarContenidoDeMemoria(&programaAImprimir,pag_marco);
				}else{
					printf("No hay ningun proceso con id %d\n",programaAImprimir);
				}
			}
		}
		else if(!strcmp(primerPalabra,flushTlb)){
			printf("Comando flush tlb activado\n");
		}
		else if(!strcmp(primerPalabra,flushMry)){
			printf("Comando flush memory activado\n");
		}
		else if(!strcmp(primerPalabra,cerrar)){
			printf("Comando close activado\n");
		}
		else{
			printf("Comando no reconocido, /help para ver comandos.\n");
		}
		free(comandoCompleto);
		comandoCompleto = malloc(50);
	}while(strcmp(primerPalabra,cerrar));
	free(comandoCompleto);
	return;
}


void* mostrarEstructura(char* programaAImprimir, void* pag_marco){
	int pidPrograma = *programaAImprimir;
	int tamanioDiccionario = dictionary_size(pag_marco);
	int i;
		printf("ID\tPag\tMarco Nro\tBit Uso\tModif\t\n");
	for(i=0;i<tamanioDiccionario;i++){
		infoPagina* marcoTemp = dictionary_get(pag_marco,&i);
		printf("%d\t %d \t    %d   \t  %d  \t  %d  \t  %d \t\n",pidPrograma,i,marcoTemp->nroMarco,marcoTemp->bit_uso,marcoTemp->modif);
	}
}

void* mostrarContenidoDeMemoria(char* programaAImprimir, void* pag_marco){
	int pidPrograma = *programaAImprimir;
	int tamanioDiccionario = dictionary_size(pag_marco);
	int i;
	int posicioDeMemoria;
	for(i=0;i<tamanioDiccionario;i++){
		infoPagina* marcoTemp = dictionary_get(pag_marco,&i);
		posicioDeMemoria = ((marcoTemp->nroMarco)*umcConfg.configuracionUMC.MARCO_SIZE);
		int j;
		printf("ID: %d PAG: %d MARCO: %d\n",pidPrograma,i,marcoTemp->nroMarco);
		for(j=0;j<vecesRepetir;j++){
			char tempHexa;
			int tempOffset = j;
			memcpy(&tempHexa,(memoriaPrincipal +posicioDeMemoria + tempOffset),1);
			/*int resultado = strtol(tempHexa,NULL,16);
			char resultadoChar = resultado;*/
			//char tempRes = (char)*tempHexa;
			/*if(((tempHexa >= '0') && (tempHexa <= 'Z')))
				printf("%c  ",tempHexa);
			else*/
			if(j%15==0&&j>1){
				printf("\n");
			}
			printf("%hhX\t",tempHexa);
		}
		printf("\n");
	}
}




