
#include <sockets/header.h>
#include <sockets/basicFunciones.h>
#include <string.h>
#include <commons/collections/dictionary.h>
#include <parser/parser.h>
static const int CONTENIDO_VARIABLE = 20;
static const int POSICION_MEMORIA = 0x10;
typedef u_int32_t t_puntero;
typedef u_int32_t t_size;
typedef u_int32_t t_puntero_instruccion;
typedef char t_nombre_variable;
typedef int t_valor_variable;
typedef t_nombre_variable* t_nombre_semaforo;
typedef t_nombre_variable* t_nombre_etiqueta;
typedef t_nombre_variable* t_nombre_compartida;
typedef t_nombre_variable* t_nombre_dispositivo;

t_dictionary* nroInst_offset;
int nroInstruccion ;


void inicialzarPrograma(t_dictionary* diccionarioInstrucciones) {
	nroInstruccion = 0;
	nroInst_offset = diccionarioInstrucciones;
}

//DefinirVariable
t_puntero vardef(t_nombre_variable var) {
	int* nroInst = malloc(sizeof(int));
	int* tamanioInst = malloc(sizeof(int));
	*tamanioInst = 12;
	*nroInst = nroInstruccion;
	dictionary_put(nroInst_offset,nroInst,tamanioInst);
	(*nroInst)++;
	return POSICION_MEMORIA;
}

//ObtenerPosicionVariable
t_puntero getvarpos(t_nombre_variable var) {

}

//Dereferenciar , ya esta lista
t_valor_variable derf(t_puntero puntero_var) {

}

//Asignar
void asignar(t_puntero puntero_var, t_valor_variable valor) {

   return;
}

//ObtenerValorCompartida
t_valor_variable getglobalvar(t_nombre_compartida var) {
	return CONTENIDO_VARIABLE;
}

//AsignarValorCompartida
t_valor_variable setglobalvar(t_nombre_compartida var, t_valor_variable valor) {
	return CONTENIDO_VARIABLE;
}

//IrALabel
t_puntero_instruccion goint(t_nombre_etiqueta etiqueta) {
	return POSICION_MEMORIA;
}

//LlamarFuncion
t_puntero_instruccion fcall(t_nombre_etiqueta etiqueta, t_puntero funcion,
		t_puntero_instruccion pinst) {
	return POSICION_MEMORIA;
}

//Retornar
t_puntero_instruccion retornar(t_valor_variable retorno) {
	return POSICION_MEMORIA;
}

//Imprimir
int imprimir(t_valor_variable var) {
	return CONTENIDO_VARIABLE;
	return 0;
}

//ImprimirTexto
int imptxt(char* aimpprimir) {
	return CONTENIDO_VARIABLE;
}

//EntradaSalida
int ionotif(t_nombre_dispositivo ioname, int tiempo) {
	return CONTENIDO_VARIABLE;
}

//wait
int wait(t_nombre_semaforo semf) {
	return CONTENIDO_VARIABLE;
}

//signal
int signal(t_nombre_semaforo semf) {
	return CONTENIDO_VARIABLE;
}

