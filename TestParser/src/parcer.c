#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <commons/log.h>
#include <commons/txt.h>
#include <commons/string.h>
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

	FILE *programaANSISOP;//puntero archivo
	programaANSISOP = fopen("facil.ansisop","r");//nombre archivo
	char* linea;//linea a analizar
	if(programaANSISOP){//chekea q el archivo se halla abierto correctamente
		fgets(linea,1024,programaANSISOP);
		do
		{
			printf("%s\n",linea);
			//analizadorLinea(linea,&functions,&kernel_functions);
		}while(fgets(linea,120,programaANSISOP) != NULL);
		linea = NULL;
	printf("Fin de Archivo");
	//txt_close_file(programaANSISOP);
	}
	else
	{
		printf("No se pudo leer el archivo");
	}
	return 0;
}
