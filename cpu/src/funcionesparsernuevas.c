//#include "funcionesparsernuevas.h"
#include <sockets/header.h>
#include <sockets/basicFunciones.h>
#include <string.h>
#include <stdlib.h>

#include "estructurasCPU.h"
static const int CONTENIDO_VARIABLE = 20;
static const int POSICION_MEMORIA = 0x10;
typedef direccionMemoria* t_puntero;
typedef int t_size;
typedef	int t_puntero_instruccion;
typedef char t_nombre_variable;
typedef int* t_valor_variable;
typedef t_nombre_variable* t_nombre_semaforo;
typedef t_nombre_variable* t_nombre_etiqueta;
typedef t_nombre_variable* t_nombre_compartida;
typedef t_nombre_variable* t_nombre_dispositivo;
almUMC calculoDeDedireccionAlmalcenar();

extern int tamanioPaginaUMC;
int socketMemoria;
int socketNucleo;
extern pcb_t* pcb_actual;
direccionMemoria* ultimaDireccion;//Tenporal hasta q tengamos stack

void inicialzarParser(int socketMem ,int socketNuc) {
	socketMemoria = socketMem;
	socketNucleo = socketNuc;
	ultimaDireccion = NULL;
}
//DefinirVariable
t_puntero vardef(t_nombre_variable var) {
	printf("Se difinio variable: %s\n", var);
	direccionMemoria* temp = malloc(sizeof(direccionMemoria));
	almUMC aAlmacenar = calculoDeDedireccionAlmalcenar();
	aAlmacenar.valor = 0;
	enviarStream(socketMemoria, 53, sizeof(almUMC), &aAlmacenar);
	ultimaDireccion->offset = aAlmacenar.offset;
	ultimaDireccion->pagina = aAlmacenar.pagina;
	ultimaDireccion->tamanio = aAlmacenar.tamanio;
	return temp;
}

//ObtenerPosicionVariable
t_puntero getvarpos(t_nombre_variable var) {
	printf("Se llama obtener posicion variable : %s\n", var);
	direccionMemoria* solicitarUMC = malloc(sizeof(direccionMemoria));
	//Se necesita stack
	solicitarUMC->pagina = 0; //obtener del pcb
	solicitarUMC->offset = 0; //obtener del pcb
	solicitarUMC->tamanio = 0; //obtener del pcb

	return solicitarUMC;
}

//Dereferenciar , ya esta lista
t_valor_variable derf(t_puntero puntero_var) {
	printf("Se llama desreferenciar");
	enviarStream(socketMemoria, 52, sizeof(direccionMemoria), puntero_var);
	void* valorRecibido;

	if (*leerHeader(socketMemoria) == 200) {
		valorRecibido = recibirStream(socketMemoria, sizeof(int));
	}

	return valorRecibido;
}

//Asignar
void asignar(t_puntero puntero_var, t_valor_variable valor) {
	almUMC temp;
	temp.pagina = (*puntero_var).pagina;
	temp.offset = (*puntero_var).offset;
	temp.tamanio = (*puntero_var).tamanio;
	printf("Se asigna una variable a un valor");
    int valorNuevo = *valor;
	temp.valor= valorNuevo;
	enviarStream(socketMemoria,53,sizeof(almUMC),&temp);
	free(puntero_var);
   return;
}

//ObtenerValorCompartida
t_valor_variable getglobalvar(t_nombre_compartida var) {
	printf("Se obtiene una variable global");

	return CONTENIDO_VARIABLE;
}

//AsignarValorCompartida
t_valor_variable setglobalvar(t_nombre_compartida var, t_valor_variable valor) {
	printf("Se setea una variable global");
	return CONTENIDO_VARIABLE;
}

//IrALabel
t_puntero_instruccion goint(t_nombre_etiqueta etiqueta) {
	printf("Se va a una instruccion");
	return POSICION_MEMORIA;
}

//LlamarFuncion
t_puntero_instruccion fcall(t_nombre_etiqueta etiqueta, t_puntero funcion,
		t_puntero_instruccion pinst) {
	printf("Se llama a una funcion");
	return POSICION_MEMORIA;
}

//Retornar
t_puntero_instruccion retornar(t_valor_variable retorno) {
	printf("Se retorna un puntero al contecto anterior");
	return POSICION_MEMORIA;
}

//Imprimir
int imprimir(t_valor_variable var) {
	printf("Se imprimi algo");
	enviarStream(socketNucleo,9,sizeof(int),var);
	return CONTENIDO_VARIABLE;
}

//ImprimirTexto
int imptxt(char* aimpprimir) {
	printf("nucleo manda a imprimir");
	return CONTENIDO_VARIABLE;
}

//EntradaSalida
int ionotif(t_nombre_dispositivo ioname, int tiempo) {
	printf("nucleo se soliciona disp de entrada y salida");
	return CONTENIDO_VARIABLE;
}

//wait
int wait(t_nombre_semaforo semf) {
	printf("Se llama a la funcion wait");
	return CONTENIDO_VARIABLE;
}

//signal
int signal(t_nombre_semaforo semf) {
	printf("Se llama a la funcion wait");
	return CONTENIDO_VARIABLE;
}

almUMC calculoDeDedireccionAlmalcenar(){
	almUMC aAlmacenar;
	int paginasDisponibles = pcb_actual->paginasDisponible;
	if(ultimaDireccion==NULL){
		int tamanioIC = dictionary_size(pcb_actual->indice_codigo);
		//primero hay q hacer un stack empty, xq en estack estaria la ultima posicion
		//como no hay stack, me fijo primero donde se almaceno la ultima linea de codigo
		int ultimoElemento = tamanioIC-1;
		direccionMemoria* ultimaDireccionUtil = dictionary_get(pcb_actual->indice_codigo,ultimoElemento);
		//DA = direccion anterior
		int paginaDA = ultimaDireccionUtil->pagina;
		int offsetDA = ultimaDireccionUtil->offset;
		int tamanioDA = ultimaDireccionUtil->tamanio;
		int paginaNueva = paginaDA;
		int offsetNuevo = offsetDA + tamanioDA;
		int tamanioNuevo = sizeof(int);
		if(offsetNuevo+tamanioNuevo>tamanioNuevo){
			paginaNueva++;
			offsetNuevo=0;
		}
		aAlmacenar.offset = offsetNuevo;
		aAlmacenar.pagina = paginaNueva;
		aAlmacenar.tamanio = tamanioNuevo;
	}else{
		int paginaDA = ultimaDireccion->pagina;
		int offsetDA = ultimaDireccion->offset;
		int tamanioDA = ultimaDireccion->tamanio;
		int paginaNueva = paginaDA;
		int offsetNuevo = offsetDA + tamanioDA;
		int tamanioNuevo = sizeof(int);
		if(offsetNuevo+tamanioNuevo>tamanioNuevo){
			paginaNueva++;
			offsetNuevo=0;
		}
		aAlmacenar.offset = offsetNuevo;
		aAlmacenar.pagina = paginaNueva;
		aAlmacenar.tamanio = tamanioNuevo;
	}
	return aAlmacenar;
}
