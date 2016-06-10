//#include "funcionesparsernuevas.h"
#include <sockets/header.h>
#include <sockets/basicFunciones.h>
#include <string.h>
#include <stdlib.h>
#include <commons/log.h>
#include "estructurasCPU.h"
static const int CONTENIDO_VARIABLE = 20;
static const int POSICION_MEMORIA = 0x10;
int finPrograma;

almUMC calculoDeDedireccionAlmalcenar();
void agregarVariableStack(almUMC aAlmacenar,char var);

direccionMemoria* obtenerPosicionStack(char var);
//Tipos de datos
typedef u_int32_t t_puntero;
typedef u_int32_t t_size;
typedef u_int32_t t_puntero_instruccion;

typedef char t_nombre_variable;
typedef int t_valor_variable;

typedef t_nombre_variable* t_nombre_semaforo;
typedef t_nombre_variable* t_nombre_etiqueta;
typedef  t_nombre_variable* t_nombre_compartida;
typedef  t_nombre_variable* t_nombre_dispositivo;

extern t_log* logCpu;
extern int tamanioPaginaUMC;
int socketMemoria;
int socketNucleo;
extern pcb_t* pcb_actual;
direccionMemoria* ultimaDireccion;//Tenporal hasta q tengamos stack

void inicialzarParser(int socketMem ,int socketNuc) {
	socketMemoria = socketMem;
	socketNucleo = socketNuc;
	ultimaDireccion = malloc(sizeof(direccionMemoria));
	ultimaDireccion->pagina = 0;
	ultimaDireccion->offset = 0;
	ultimaDireccion->tamanio = 0;
}
//DefinirVariable
t_puntero vardef(t_nombre_variable var) {
	log_trace(logCpu,"Crear variable id: %d nombre: %s.",pcb_actual->PID,var);
	direccionMemoria* temp = malloc(sizeof(direccionMemoria));
	almUMC aAlmacenar = calculoDeDedireccionAlmalcenar();
	aAlmacenar.valor = 0;
	aAlmacenar.tamanio = sizeof(int);
	enviarStream(socketMemoria, 53, sizeof(almUMC), &aAlmacenar);
	//Actualizo stack
	agregarVariableStack(aAlmacenar,var);
	return NULL;
}

//ObtenerPosicionVariable
t_puntero getvarpos(t_nombre_variable var) {
	log_trace(logCpu, "Se llama obtener posicion variable : %s\n", var);
	//Obtengo posicion de memoria de la umc
	direccionMemoria* solicitarUMC = malloc(sizeof(direccionMemoria));
	solicitarUMC= obtenerPosicionStack(var);
	return solicitarUMC;
}

//Dereferenciar , ya esta lista
t_valor_variable derf(t_puntero puntero_var) {
	log_trace(logCpu, "Solicitar  a memoria comezado");
	enviarStream(socketMemoria, 52, sizeof(direccionMemoria), puntero_var);
	int* valorRecibido;
	if(*leerHeader(socketMemoria) == 200) {
		valorRecibido = recibirStream(socketMemoria, sizeof(int));
	}
	return *valorRecibido;
}

//Asignar
void asignar(t_puntero puntero_var, t_valor_variable valor) {
	almUMC temp;
	direccionMemoria* direcMemory = puntero_var;
	temp.pagina = direcMemory->pagina;
	temp.offset = direcMemory->offset;
	temp.tamanio = direcMemory->pagina;
	temp.valor= valor;
	log_trace(logCpu, "Almacenar en UMC nuevo valor");
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
	enviarStream(socketNucleo,9,sizeof(int),&var);
	return CONTENIDO_VARIABLE;
}

//ImprimirTexto
int imptxt(char* aimpprimir) {
	printf("nucleo manda a imprimir");
	return CONTENIDO_VARIABLE;
}

void fin(){

	finPrograma=1;
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

//FUNCIONES PARA STACK-----------------------------------------------------------------------------------

almUMC calculoDeDedireccionAlmalcenar(){
	almUMC aAlmacenar;
	//Obtego stack
	stack* stackActual = dictionary_get(pcb_actual->indice_stack,*(pcb_actual->SP));
	//Me fijo si al agregarle el offset a la ultima direccion, aumenta el del tamanio de la pagina
	//Si aumenta paso a otra pagina
	if (ultimaDireccion->offset + sizeof(int) > tamanioPaginaUMC) {
		ultimaDireccion->pagina = ultimaDireccion->pagina + 1; // Aumento una pagina
		ultimaDireccion->offset = 0; //resteo el offset xq es una pagina nueva
	}
	else{
	ultimaDireccion->offset = ultimaDireccion->offset + sizeof(int);//sino me paso aumeto offset
	}
	aAlmacenar.pagina = ultimaDireccion->pagina; // seteo ultima direccion a alamcenar
	aAlmacenar.offset = ultimaDireccion->offset;
	return aAlmacenar;
}

void agregarVariableStack(almUMC aAlmacenar,char var){

	//Obtego stack
	stack* stackActual = dictionary_get(pcb_actual->indice_stack,*(pcb_actual->SP));
	//Obtengo lista de variables
	t_list* listaStackActual = stackActual->vars;
	//Creo direccion
	direccionStack* aGuardar = malloc(sizeof(direccionStack));
	aGuardar->id;
	aGuardar->lugarUMC.pagina = aAlmacenar.pagina;
	aGuardar->lugarUMC.offset = aAlmacenar.offset;
	aGuardar->lugarUMC.tamanio = aAlmacenar.tamanio;
	//agrego variable
	list_add(listaStackActual,aGuardar);
	log_trace(logCpu,"Variable agregada al stack");
}

direccionMemoria* obtenerPosicionStack(char var){
	//Obtengo lista variables de la posicion del stack
	//Obtego stack
	stack* stackActual = dictionary_get(pcb_actual->indice_stack,*(pcb_actual->SP));
	//Obtengo lista de variables
	t_list* listaStackActual = stackActual->vars;
	int encontrado = 0;
	int i = 0;
	while(!encontrado && i < list_size(listaStackActual)){
		direccionStack* tempDirec = list_get(listaStackActual,i);
		if(tempDirec->id == var){
			return &(tempDirec->lugarUMC);
		}
	}
	return NULL;
}





