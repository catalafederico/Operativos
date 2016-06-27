
//#include <sockets/header.h>
#include <sockets/basicFunciones.h>
#include <string.h>
#include <commons/collections/dictionary.h>
#include <parser/parser.h>
#include <commons/collections/list.h>
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
int esVariable;
int esFuncion;
char *nombreFuncion;
void inicialzarPrograma() {
	nombreFuncion = NULL;
	esVariable = 0;
	esFuncion = 0;
}

//DefinirVariable
t_puntero vardef(t_nombre_variable var) {
	// sumo un 4 ya q necesita 4 bytes la variable
	esVariable = 1;
	return POSICION_MEMORIA;
}

//ObtenerPosicionVariable
t_puntero getvarpos(t_nombre_variable var) {
	esVariable = 1;
}

//Dereferenciar , ya esta lista
t_valor_variable derf(t_puntero puntero_var) {
	esVariable = 1;
}

//Asignar
void asignar(t_puntero puntero_var, t_valor_variable valor) {
	esVariable = 1;
   return;
}

//ObtenerValorCompartida
t_valor_variable getglobalvar(t_nombre_compartida var) {
	esVariable = 1;
	return CONTENIDO_VARIABLE;
}

//AsignarValorCompartida
t_valor_variable setglobalvar(t_nombre_compartida var, t_valor_variable valor) {
	esVariable = 1;
	return CONTENIDO_VARIABLE;
}

//IrALabel
t_puntero_instruccion goint(t_nombre_etiqueta etiqueta) {
	esVariable = 1;
	return POSICION_MEMORIA;
}

//LlamarFuncion
t_puntero_instruccion fcall(t_nombre_etiqueta etiqueta, t_puntero funcion,
		t_puntero_instruccion pinst) {
	esVariable = 1;
	return POSICION_MEMORIA;
}

//Retornar
t_puntero_instruccion retornar(t_valor_variable retorno) {
	esVariable = 1;
	return POSICION_MEMORIA;
}

//Imprimir
int imprimir(t_valor_variable var) {
	esVariable = 1;
	return CONTENIDO_VARIABLE;
	return 0;
}

//ImprimirTexto
int imptxt(char* aimpprimir) {
	esVariable = 1;
	return CONTENIDO_VARIABLE;
}

//EntradaSalida
int ionotif(t_nombre_dispositivo ioname, int tiempo) {
	esVariable = 1;
	return CONTENIDO_VARIABLE;
}

//wait
void waitCPU(t_nombre_semaforo semf) {
	esVariable = 1;
	return;// CONTENIDO_VARIABLE;
}

//signal
void signalCPU(t_nombre_semaforo semf) {
	esVariable = 1;
	return;//CONTENIDO_VARIABLE;
}

//Funciones agreagadas 10/06/2016
void fin(){
	esVariable = 1;
}

void fcallNR(t_nombre_etiqueta nombre){
	esFuncion = 1;
}

int* obtenerEsVariable(){
	return &esVariable;
}
