#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
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
	programaANSISOP = fopen("facil.ansisop","r");//Archivo ejemplo
	if(programaANSISOP){
		char* linea; //aca va a almacenar la linea leida
		fgets(linea,1024,programaANSISOP);
		while(strcmp(linea,"\n")){
		//ERROR1:	analizadorLinea(strdup(linea),&functions,&kernel_functions);//CUANDO ESTA LINEA NO ESTA COMENTADA NO COMPILA
			printf("%s\n",linea);
			fgets(linea,1024,programaANSISOP);
		}
		printf("Fin de Archivo");//No lo imprime nosexq error: No source available for "0xa323120"
	}
	else
	{
		printf("No se pudo leer el archivo");
	}
	fclose(programaANSISOP);
	return 0;
}
