#include "funcionesparsernuevas.h"

static const int CONTENIDO_VARIABLE = 20;
static const int POSICION_MEMORIA = 0x10;


//DefinirVariable
t_puntero vardef(t_nombre_variable var){
	printf("Se difinio variable: ");
	return POSICION_MEMORIA;
}

//ObtenerPosicionVariable
t_puntero getvarpos(t_nombre_variable var){
	printf("Se llama obtener posicion variable");
	return POSICION_MEMORIA;
}

//Dereferenciar
t_valor_variable derf(t_puntero puntero_var){
	printf("Se llama desreferenciar");
	return CONTENIDO_VARIABLE;
}

//Asignar
void asignar(t_puntero puntero_var, t_valor_variable valor){
	printf("Se asigna una variable a un valor");
	return;
}

//ObtenerValorCompartida
t_valor_variable getglobalvar(t_nombre_compartida var){
	printf("Se obtiene una variable global");
	return CONTENIDO_VARIABLE;
}

//AsignarValorCompartida
t_valor_variable setglobalvar(t_nombre_compartida var, t_valor_variable valor){
	printf("Se setea una variable global");
	return CONTENIDO_VARIABLE;
}

//IrALabel
t_puntero_instruccion goint(t_nombre_etiqueta etiqueta){
	printf("Se va a una instruccion");
	return POSICION_MEMORIA;
}

//LlamarFuncion
t_puntero_instruccion fcall(t_nombre_etiqueta etiqueta, t_puntero funcion, t_puntero_instruccion pinst){
	printf("Se llama a una funcion");
	return POSICION_MEMORIA;
}

//Retornar
t_puntero_instruccion retornar(t_valor_variable retorno){
	printf("Se retorna un puntero al contecto anterior");
	return POSICION_MEMORIA;
}

//Imprimir
int imprimir(t_valor_variable var){
	printf("Se imprimi algo");
	return CONTENIDO_VARIABLE;
}

//ImprimirTexto
int imptxt(char* aimpprimir){
	printf("nucleo manda a imprimir");
	return CONTENIDO_VARIABLE;
}

//EntradaSalida
int ionotif(t_nombre_dispositivo ioname, int tiempo){
	printf("nucleo se soliciona disp de entrada y salida");
	return CONTENIDO_VARIABLE;
}

//wait
int wait(t_nombre_semaforo semf){
	printf("Se llama a la funcion wait");
	return CONTENIDO_VARIABLE;
}

//signal
int signal(t_nombre_semaforo semf){
	printf("Se llama a la funcion wait");
	return CONTENIDO_VARIABLE;
}
















