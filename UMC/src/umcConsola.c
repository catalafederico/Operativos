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
#define verTlb "vertlb"
#define flushMry "fmry"
#define cerrar "close"

extern t_list* tlbCache;
extern t_dictionary* programas_ejecucion;
extern pthread_mutex_t semaforoMemoria;
extern umcNucleo umcConfg;
extern void* memoriaPrincipal;
int vecesRepetir;
t_log* dump;

void* mostrarEstructura(char* programaAImprimir, void* pag_marco);
void* mostrarContenidoDeMemoria(char* programaAImprimir, void* pag_marco);
void* flushTLB(tlb* elem);
void* flushTabla(char* id,t_dictionary* pag_infoPag);
void* mostrarTLB(tlb* elem, int i);

void consolaUMC(){
	char* comandoCompleto = malloc(50);
	char* primerPalabra;
	vecesRepetir = umcConfg.configuracionUMC.MARCO_SIZE/1;
	dump = log_create("dumps","UMC",1,LOG_LEVEL_INFO);
	do{
		printf("Ingrese un comando.(help para ver comandos.) \n");
		fgets(comandoCompleto,50,stdin);
		int i;
		i = strlen(comandoCompleto)-1;
		if( comandoCompleto[ i ] == '\n')
			  comandoCompleto[i] = '\0';
		string_trim(&comandoCompleto);
		char ** todasLasPalabras = string_split(comandoCompleto, " ");
		primerPalabra = todasLasPalabras[0];
		printf("Ha ingresado:\n\t------%s-------\n",comandoCompleto);
		pthread_mutex_lock(&semaforoMemoria);
		if(!strcmp(primerPalabra,ayuda)){
			printf("Comando ayuda activado\n");
			printf("Comando delay: %s\n", retardo);
			printf("Comando dump struct memory: %s\n", dumpStrMemory);
			printf("Comando dump contenido memory: %s\n", dumpContMemory);
			printf("Comando flush tlb: %s\n", flushTlb);
			printf("Comando flush memory: %s\n", flushMry);
			printf("Comando ver tlb: %s\n", verTlb);
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
			printf("TLB INICIAL\n");
			int i;
			for(i=0;i<list_size(tlbCache);i++){
				mostrarTLB(list_get(tlbCache,i),i);
			}
			printf("TLB flush comenzado\n");
			list_clean_and_destroy_elements(tlbCache,flushTLB);
			printf("TLB flush finalizado\n");
			printf("TLB FINAL\n");
			for(i=0;i<list_size(tlbCache);i++){
				mostrarTLB(list_get(tlbCache,i),i);
			}
		}
		else if(!strcmp(primerPalabra,flushMry)){
			printf("Flush memory comenzado\n");
			dictionary_iterator(programas_ejecucion,flushTabla);
			printf("Flush memory terminado\n");
		}
		else if(!strcmp(primerPalabra,verTlb)){
			printf("TLB mostrando\n");
			int i;
			for(i=0;i<list_size(tlbCache);i++){
				mostrarTLB(list_get(tlbCache,i),i);
			}
			//list_iterate(tlbCache,mostrarTLB);
			printf("TLB mostrado completo\n");
		}
		else if(!strcmp(primerPalabra,cerrar)){
			printf("Comando close activado\n");
		}
		else{
			printf("Comando no reconocido, /help para ver comandos.\n");
		}
		free(comandoCompleto);
		pthread_mutex_unlock(&semaforoMemoria);
		comandoCompleto = malloc(50);
	}while(strcmp(primerPalabra,cerrar));
	free(comandoCompleto);
	return;
}


void* mostrarEstructura(char* programaAImprimir, void* pag_marco){
	int pidPrograma = *programaAImprimir;
	int tamanioDiccionario = dictionary_size(pag_marco);
	int i;
	log_info(dump,"ID\tPag\tMarco Nro\tBit Uso\tModif\t");
	for(i=0;i<tamanioDiccionario;i++){
		infoPagina* marcoTemp = dictionary_get(pag_marco,&i);
		//sprintf(aImprimir + strlen(aImprimir) , "%d\t %d \t    %d   \t  %d  \t  %d  \t\n",pidPrograma,i,marcoTemp->nroMarco,marcoTemp->bit_uso,marcoTemp->modif);
		log_info(dump,"%d\t %d \t    %d   \t  %d  \t  %d  \t",pidPrograma,i,marcoTemp->nroMarco,marcoTemp->bit_uso,marcoTemp->modif);
	}
}

void* mostrarContenidoDeMemoria(char* programaAImprimir, void* pag_marco) {
	int pidPrograma = *programaAImprimir;
	int tamanioDiccionario = dictionary_size(pag_marco);
	int i;
	int posicioDeMemoria;
	for (i = 0; i < tamanioDiccionario; i++) {
		infoPagina* marcoTemp = dictionary_get(pag_marco, &i);
		if (marcoTemp->nroMarco == -1) {
			printf("ID: %d PAG: %d MARCO: %d\n", pidPrograma, i,
								marcoTemp->nroMarco);
			printf("Pagina no se encuentra en memoria\n\n");
		} else {
			posicioDeMemoria = ((marcoTemp->nroMarco)
					* umcConfg.configuracionUMC.MARCO_SIZE);
			int j;
			printf("ID: %d PAG: %d MARCO: %d\n", pidPrograma, i,
					marcoTemp->nroMarco);
			for (j = 0; j < vecesRepetir; j++) {
				char tempHexa;
				int tempOffset = j;
				memcpy(&tempHexa,
						(memoriaPrincipal + posicioDeMemoria + tempOffset), 1);
				if (j % 15 == 0 && j > 1) {
					printf("\n");
				}
				printf("%hhX\t", tempHexa);
			}
			printf("\n");
		}
	}
}


void* flushTLB(tlb* elem){
	free(elem);
}

void* flushTabla(char* id,t_dictionary* pag_infoPag){
	int tamanio = dictionary_size(pag_infoPag);
	int i;
	for(i=0;i<tamanio; i++){
		infoPagina* infoPagina = dictionary_get(pag_infoPag,&i);
		if(infoPagina->nroMarco != -1){
			printf("ID: %d\tPAG: %d\tMODVIEJO: %d",*id,i,infoPagina->modif);
			infoPagina->modif = 1;
			printf("\tMODNUEVO: %d\n",infoPagina->modif);
		}
		else{
			printf("ID: %d\tPAG: %d\t No se encuentra en memoria\n");
		}
	}
}

void* mostrarTLB(tlb* elem, int i){
	printf("POS: %d\tID: %d\tPAG: %d\tMARCO: %d\tUSO: %d\tMOD: %d\n",i,elem->idProg,elem->pag,elem->marco->nroMarco,elem->marco->bit_uso,elem->marco->modif);
}

