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

#define ayuda "help"
#define retardo "delay"
#define dumpStrMemory "dstrmry"
#define dumpContMemory "dcontmry"
#define flushTlb "ftlb"
#define flushMry "fmry"
#define cerrar "close"

extern umcNucleo umcConfg;

void consolaUMC(){
	char* comando = malloc(30);
	do{
		printf("Ingrese un comando.(help para ver comandos.) \n");
		scanf("%s", comando);
		if(!strcmp(comando,ayuda)){
			printf("Comando ayuda activado\n");
			printf("Comando delay: %s\n", retardo);
			printf("Comando dump struct memory: %s\n", dumpStrMemory);
			printf("Comando dump contenido memory: %s\n", dumpContMemory);
			printf("Comando flush tlb: %s\n", flushTlb);
			printf("Comando flush memory: %s\n", flushMry);
			printf("Comando cerrar: %s\n", cerrar);
		}
		else if(!strcmp(comando,retardo)){
			printf("Comando delay activado\n");
		}
		else if(!strcmp(comando,dumpStrMemory)){
			printf("Comando dump struct memory activado\n");
		}
		else if(!strcmp(comando,dumpContMemory)){
			printf("Comando dump contenido memory activado\n");
		}
		else if(!strcmp(comando,flushTlb)){
			printf("Comando flush tlb activado\n");
		}
		else if(!strcmp(comando,flushMry)){
			printf("Comando flush memory activado\n");
		}
		else if(!strcmp(comando,cerrar)){
			printf("Comando close activado\n");
		}
		else{
			printf("Comando no reconocido, /help para ver comandos.\n");
		}
	}while(strcmp(comando,cerrar));
	free(comando);
	return;
}
