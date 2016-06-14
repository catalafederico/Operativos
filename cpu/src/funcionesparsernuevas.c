//#include "funcionesparsernuevas.h"
#include <sockets/header.h>
#include <sockets/basicFunciones.h>
#include <string.h>
#include <stdlib.h>
#include <commons/log.h>
#include "estructurasCPU.h"


//Tipos de datos

direccionMemoria* obtenerPosicionStack(char var);
static const int CONTENIDO_VARIABLE = 20;
static const int POSICION_MEMORIA = 0x10;
int finPrograma;
int esFuncion;
typedef u_int32_t t_puntero;
typedef u_int32_t t_size;
typedef u_int32_t t_puntero_instruccion;

typedef char t_nombre_variable;
typedef int t_valor_variable;

typedef t_nombre_variable* t_nombre_semaforo;
typedef t_nombre_variable* t_nombre_etiqueta;
typedef t_nombre_variable* t_nombre_compartida;
typedef t_nombre_variable* t_nombre_dispositivo;

extern t_log* logCpu;
extern int tamanioPaginaUMC;
int socketMemoria;
int socketNucleo;
extern pcb_t* pcb_actual;
direccionMemoria* ultimaDireccion; //Tenporal hasta q tengamos stack
t_list* lista_argumentos_temporales;
int esFuncion;

//Prototipos

void inicialzarParser(int socketMem, int socketNuc);
t_puntero vardef(t_nombre_variable var);
t_puntero getvarpos(t_nombre_variable var);
t_valor_variable derf(t_puntero puntero_var);
almUMC calculoDeDedireccionAlmalcenar();
void agregarVariableStack(almUMC aAlmacenar, char var);
void asignar(t_puntero puntero_var, t_valor_variable valor);
t_valor_variable getglobalvar(t_nombre_compartida var);
t_valor_variable setglobalvar(t_nombre_compartida var, t_valor_variable valor);
t_puntero_instruccion goint(t_nombre_etiqueta etiqueta);
void fcall(t_nombre_etiqueta etiqueta, t_puntero funcion);
void retornar(t_valor_variable retorno);
int imprimir(t_valor_variable var);
int imptxt(char* aimpprimir);
void fin();
int ionotif(t_nombre_dispositivo ioname, int tiempo);
int wait(t_nombre_semaforo semf);
int signal(t_nombre_semaforo semf);
almUMC calculoDeDedireccionAlmalcenar();
void agregarVariableStack(almUMC aAlmacenar, char var);
direccionMemoria* obtenerPosicionStack(char var);

void inicialzarParser(int socketMem, int socketNuc) {
	socketMemoria = socketMem;
	socketNucleo = socketNuc;
	ultimaDireccion = malloc(sizeof(direccionMemoria));
	ultimaDireccion->pagina = 0;
	ultimaDireccion->offset = 0;
	ultimaDireccion->tamanio = 0;
	esFuncion = 0;
	lista_argumentos_temporales = list_create();
}
//DefinirVariable
t_puntero vardef(t_nombre_variable var) {
	log_trace(logCpu, "Crear variable id: %d nombre: %c.", pcb_actual->PID,
			var);
	direccionMemoria* temp = malloc(sizeof(direccionMemoria));
	almUMC aAlmacenar = calculoDeDedireccionAlmalcenar();
	aAlmacenar.valor = 0;
	aAlmacenar.tamanio = sizeof(int);
	direccionMemoria* direc_arg = malloc(sizeof(direccionMemoria));
	direc_arg-> offset = aAlmacenar.offset;
	direc_arg->pagina = aAlmacenar.pagina;
	direc_arg->tamanio = aAlmacenar.tamanio;
	if (var >= '0' && var <= '9') {
		//si son estos caracteres se esta almacenando un argumento
		//se alamcena en la lista argumentos
		// el renglon del stack se aumenta en fcall
		stack* tempStack = dictionary_get(pcb_actual->indice_stack,pcb_actual->SP);
		t_list* tempList = tempStack->args;
		int posicion = var - '0';
		direccionMemoria* direc_arg = malloc(sizeof(direccionMemoria));
		direc_arg-> offset = aAlmacenar.offset;
		direc_arg->pagina = aAlmacenar.pagina;
		direc_arg->tamanio = aAlmacenar.tamanio;
		list_add(tempList,direc_arg);
	} else {
		//Si no es una variable si se va a almacenar en la lista de variables del stack
		aAlmacenar.valor = 0;
		aAlmacenar.tamanio = sizeof(int);
		enviarStream(socketMemoria, 53, sizeof(almUMC), &aAlmacenar);
		send(socketMemoria, pcb_actual->PID, sizeof(int), 0);
		//Actualizo stack
		agregarVariableStack(aAlmacenar, var);
	}
	return direc_arg;
}

//ObtenerPosicionVariable
t_puntero getvarpos(t_nombre_variable var) {
	direccionMemoria* solicitarUMC = malloc(sizeof(direccionMemoria));
	if (var >= '0' && var <= '9') {
		//si esta entre esos caracteres significa q estamos dentro de una funcion y ya esta declarada
		//entonces obtenemos variables de la lista de argumentos del stack
		stack* tempStack = dictionary_get(pcb_actual->indice_stack,pcb_actual->SP);
		t_list* tempList = tempStack->args;
		int posicion = var - '0';
		solicitarUMC = list_get(tempList,posicion);
	} else {
		log_trace(logCpu, "Se llama obtener posicion variable : %c\n", var);
		//Obtengo posicion de memoria de la umc
		solicitarUMC = obtenerPosicionStack(var);
	}
	return solicitarUMC;
}

//Dereferenciar , ya esta lista
t_valor_variable derf(t_puntero puntero_var) {
	log_trace(logCpu, "Solicitar  a memoria comezado");
	enviarStream(socketMemoria, 52, sizeof(direccionMemoria), puntero_var);
	send(socketMemoria, pcb_actual->PID, sizeof(int), 0);
	int* valorRecibido;
	valorRecibido = recibirStream(socketMemoria, sizeof(int));
	return *valorRecibido;
}

//Asignar
void asignar(t_puntero puntero_var, t_valor_variable valor) {
		almUMC temp;
		direccionMemoria* direcMemory = puntero_var;
		temp.pagina = direcMemory->pagina;
		temp.offset = direcMemory->offset;
		temp.tamanio = direcMemory->tamanio;
		temp.valor = valor;
		log_trace(logCpu, "Almacenar en UMC nuevo valor");
		enviarStream(socketMemoria, 53, sizeof(almUMC), &temp);
		send(socketMemoria, pcb_actual->PID, sizeof(int), 0);
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
void fcall(t_nombre_etiqueta etiqueta, t_puntero funcion) {
	//SE LLAMA UNA FUNCION
	//Creo una posicion de stack nueva
	stack* nuevaPosicion = malloc(sizeof(int));
	//Creo lista de argumentos
	nuevaPosicion->args = list_create();
	//Guardo
	list_add_all(nuevaPosicion->args,lista_argumentos_temporales);
	list_clean(lista_argumentos_temporales);
	//Creo lista de variables
	nuevaPosicion->vars = list_create();
	//Guardo prozima instruccion
	nuevaPosicion->pos_ret = malloc(sizeof(int));
	*(nuevaPosicion->pos_ret) = *(pcb_actual->PC);
	int cantidadDeFunciones = list_size(pcb_actual->indice_funciones);
	int i = 0;
	for (i= 0 ; i< cantidadDeFunciones; i++){
		funcion_sisop* temp = list_get(pcb_actual->indice_funciones,i);
		if(strcmp(temp->funcion,etiqueta)==0){
			*(pcb_actual->PC) = *(temp->posicion_codigo);
			break;
		}
	}
	//
	*(pcb_actual->SP) = *(pcb_actual->SP) + 1;
	int* nuevaPos = malloc(sizeof(int));
	*nuevaPos = *(pcb_actual->SP);
	nuevaPosicion->memoriaRetorno = funcion;
	dictionary_put(pcb_actual->indice_stack,nuevaPos,nuevaPosicion);
	return;
}

//Retornar
void retornar(t_valor_variable retorno) {
	printf("Se retorna un puntero al contecto anterior");
	stack* stackActual = dictionary_get(pcb_actual->indice_stack,(pcb_actual->SP));
	direccionMemoria* retornoDireccion = stackActual->memoriaRetorno;
	asignar(retornoDireccion,retorno);
	*(pcb_actual->PC) = *(stackActual->pos_ret);
	dictionary_remove(pcb_actual->indice_stack,(pcb_actual->SP));//SACO RENGLON DEL DICCIONARIO STACK
	*(pcb_actual->SP) = *(pcb_actual->SP) - 1;
	//BORRAR RENGLON DE MEMORA

	return;
}

//Imprimir
int imprimir(t_valor_variable var) {
	printf("Se imprimi algo");
	enviarStream(socketNucleo, 9, sizeof(int), &var);
	return CONTENIDO_VARIABLE;
}

//ImprimirTexto
int imptxt(char* aimpprimir) {
	printf("nucleo manda a imprimir");
	return CONTENIDO_VARIABLE;
}

void fin() {

	finPrograma = 1;
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

void fcallNR(t_nombre_etiqueta nombre) {
	esFuncion = 1;
}

//FUNCIONES PARA STACK-----------------------------------------------------------------------------------

almUMC calculoDeDedireccionAlmalcenar() {
	almUMC aAlmacenar;
	//Obtego stack
	if (dictionary_is_empty(pcb_actual->indice_stack)) {
		int nroUltmaDireccionDeCodigo = dictionary_size(
				pcb_actual->indice_codigo) - 1;
		direccionMemoria* ultimaDireccionCodigo = dictionary_get(
				pcb_actual->indice_codigo, &nroUltmaDireccionDeCodigo);
		ultimaDireccion->offset = 0;
		ultimaDireccion->pagina = ultimaDireccionCodigo->pagina + 1;
	} else {
		stack* stackActual = dictionary_get(pcb_actual->indice_stack,
				(pcb_actual->SP));
		//Me fijo si al agregarle el offset a la ultima direccion, aumenta el del tamanio de la pagina
		//Si aumenta paso a otra pagina
		if (ultimaDireccion->offset + sizeof(int) > tamanioPaginaUMC) {
			ultimaDireccion->pagina = ultimaDireccion->pagina + 1; // Aumento una pagina
			ultimaDireccion->offset = 0; //resteo el offset xq es una pagina nueva
		} else {
			ultimaDireccion->offset = ultimaDireccion->offset + sizeof(int); //sino me paso aumeto offset
		}
	}
	aAlmacenar.pagina = ultimaDireccion->pagina; // seteo ultima direccion a alamcenar
	aAlmacenar.offset = ultimaDireccion->offset;
	return aAlmacenar;
}

void agregarVariableStack(almUMC aAlmacenar, char var) {

	//Obtego stack
	stack* stackActual;
	if(dictionary_is_empty(pcb_actual->indice_stack)){
		int nroUltmaDireccionDeCodigo = dictionary_size(pcb_actual->indice_codigo)-1;
		direccionMemoria* ultimaDireccionCodigo =  dictionary_get(pcb_actual->indice_codigo,&nroUltmaDireccionDeCodigo);
		stack* nuevoStack= malloc(sizeof(stack));
		nuevoStack->args = NULL;
		nuevoStack->memoriaRetorno = NULL;
		nuevoStack->pos_ret = NULL;
		nuevoStack->vars = list_create();
		int* startPCB = malloc(sizeof(int));
		startPCB = 0;
		dictionary_put(pcb_actual->indice_stack,&startPCB,nuevoStack);
		stackActual = nuevoStack;
	}
	else{
		stackActual = dictionary_get(pcb_actual->indice_stack,
				(pcb_actual->SP));
	}
		//Obtengo lista de variables
		t_list* listaStackActual = stackActual->vars;
		//Creo direccion
		direccionStack* aGuardar = malloc(sizeof(direccionStack));
		aGuardar->id = var;
		aGuardar->lugarUMC.pagina = aAlmacenar.pagina;
		aGuardar->lugarUMC.offset = aAlmacenar.offset;
		aGuardar->lugarUMC.tamanio = aAlmacenar.tamanio;
		list_add(listaStackActual, aGuardar);
		//agrego variable
	log_trace(logCpu, "Variable agregada al stack");
}

direccionMemoria* obtenerPosicionStack(char var) {
	//Obtengo lista variables de la posicion del stack
	//Obtego stack
	stack* stackActual = dictionary_get(pcb_actual->indice_stack,
			(pcb_actual->SP));
	//Obtengo lista de variables
	t_list* listaStackActual = stackActual->vars;
	int encontrado = 0;
	int i = 0;
	while (!encontrado && i < list_size(listaStackActual)) {
		direccionStack* tempDirec = list_get(listaStackActual, i);
		if (tempDirec->id == var) {
			return &(tempDirec->lugarUMC);
		}
		i++;
	}
	return NULL;
}


void eliminarTempVars(){
	list_clean(lista_argumentos_temporales);
}

