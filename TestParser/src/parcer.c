#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <commons/log.h>
#include <parser/parser.h>
#include "funcionesparsernuevas.h"

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

int main(int argc, char **argv) {
	FILE *programaANSISOP;
	programaANSISOP = fopen("facil.ansisop","r");
	if(programaANSISOP){
		char* linea = malloc(120);
		fgets(linea,1024,programaANSISOP);
		while(strcmp(linea,"\n")){
			printf("%s\n",linea);
			analizadorLinea(strdup(linea),&functions,&kernel_functions);
			fgets(linea,1024,programaANSISOP);
		}
		printf("Fin de Archivo");
	}
	else
	{
		printf("No se pudo leer el archivo");
	}
	fclose(programaANSISOP);
	return 0;
}
