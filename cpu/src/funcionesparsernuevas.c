#include "funcionesparsernuevas.h"
#include "estructuras.h"
#include <sockets/header.h>
#include <sockets/basicFunciones.h>
#include <string.h>
static const int CONTENIDO_VARIABLE = 20;
static const int POSICION_MEMORIA = 0x10;
typedef solcUMC* t_puntero;
typedef u_int32_t t_size;
typedef u_int32_t t_puntero_instruccion;
typedef char t_nombre_variable;
typedef void* t_valor_variable;
typedef t_nombre_variable* t_nombre_semaforo;
typedef t_nombre_variable* t_nombre_etiqueta;
typedef t_nombre_variable* t_nombre_compartida;
typedef t_nombre_variable* t_nombre_dispositivo;

int socketMemoria;
int socketNucleo;
pcb* pcb_Actual;
void inicialzarParser(int socketMem ,int socketNuc) {
	socketMemoria = socketMem;
	socketNucleo = socketNuc;
}

void cambiarPCB(pcb* pcb_actual) {
	pcb_Actual = pcb_actual;

}
//DefinirVariable
t_puntero vardef(t_nombre_variable var) {
	printf("Se difinio variable: %s", var);
	solcUMC* temp = malloc(sizeof(solcUMC));
	almUMC aAlmacenar;
	aAlmacenar.pagina = 0; //calcular con el pcb;
	aAlmacenar.offset = 0; //
	aAlmacenar.tamanio = sizeof(int);
	aAlmacenar.valor = 0;
	temp->pagina=0;
	temp->offset=0;
	temp->tamanio= sizeof(int);
	enviarStream(socketMemoria, 53, sizeof(almUMC), &aAlmacenar);
	//guardar aALcenar, pagina ,offset,tamnio en el pcb
	return temp;
}

//ObtenerPosicionVariable
t_puntero getvarpos(t_nombre_variable var) {
	printf("Se llama obtener posicion variable : %s", var);
	solcUMC* solicitarUMC = malloc(sizeof(solcUMC));
	solicitarUMC->pagina = 0; //obtener del pcb
	solicitarUMC->offset = 0; //obtener del pcb
	solicitarUMC->tamanio = 0; //obtener del pcb

	return solicitarUMC;
}

//Dereferenciar , ya esta lista
t_valor_variable derf(t_puntero puntero_var) {
	printf("Se llama desreferenciar");
	enviarStream(socketMemoria, 52, sizeof(solcUMC), puntero_var);
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

