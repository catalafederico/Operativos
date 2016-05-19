/*
 ============================================================================
 Name        : swap.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <commons/config.h>
#include <commons/log.h>
#include <sockets/socketServer.h>
#include <sockets/basicFunciones.h>
#include <commons/collections/list.h>
#include <pthread.h>
#include <unistd.h>



// Estructuras

typedef struct {
	int PUERTO_ESCUCHA;
	char* NOMBRE_SWAP;
	int CANTIDAD_PAGINAS;
	int TAMANIO_PAGINA;
	int RETARDO_COMPACTACION;
} t_reg_config;

typedef struct{
	int pid;
	int cantidadDePaginas;
	int comienzo;
	struct proceso* procesoSiguiente;
}proceso;

// Funciones

t_reg_config get_config_params(void);

void crearArchivo(void);

void inicializarArchivo(void);

int conectarseConUMC(struct server servidor);

void manejarConexionesConUMC(void);

int recibirMensajes(void);

int recibirHeader(void);

void iniciar(void);

int entraProceso(proceso proceso);

proceso crearProceso(int pid, int cantidadDePaginas);

void inicializarBitMap(void);

void listarBitMap(void);

void actualizarBitMap(proceso* proceso,int comienzoDelHueco);

void eliminarDelBitMapLasPaginasDelProceso(proceso* proceso);

void insertarProceso(proceso* proceso);

int hayHuecoDondeCabeProceso(proceso* proceso);

void agregarProcesoAListaSwap(proceso* proceso);

void compactar(void);

void unirProcesos(proceso* procesoAnterior, proceso* procesoAJuntar);

void moverPaginas(proceso* procesoAJuntar,int nuevoComienzo);

void dormir(void);

//Variables globales

t_reg_config swap_configuracion;

int socketAdministradorDeMemoria;

int *bitMap;

proceso* listaSwap;

pthread_mutex_t mutex;


int main(void) {

	pthread_mutex_init(&mutex,NULL);

	//Leo el archivo de configuración
	swap_configuracion = get_config_params();

	inicializarBitMap();

	//Creo el archivo de log
	t_log* log_swap = log_create("log_swap", "Swap", false, LOG_LEVEL_INFO);

	//Archivo swap
    crearArchivo();
    inicializarArchivo();


    proceso proceso2 = crearProceso(1,23);
    /*int comienzaHueco = hayHuecoDondeCabeProceso(proceso2);
    printf("Hueco comienza en:%d \n",comienzaHueco);
    actualizarBitMap(proceso2,comienzaHueco);
    listarBitMap();*/
    insertarProceso(&proceso2);
    //printf("Pid: %d \n", listaSwap->pid);

    proceso proceso1 = crearProceso(2,47);
    insertarProceso(&proceso1);
    while(listaSwap != NULL){
    	printf("Pid: %d \n", listaSwap->pid);
    	listaSwap = listaSwap->procesoSiguiente;
    }

    eliminarDelBitMapLasPaginasDelProceso(&proceso1);
    listarBitMap();

    /*int comienzaHueco1 = hayHuecoDondeCabeProceso(proceso1);
    printf("Hueco comienza en:%d \n",comienzaHueco1);
    actualizarBitMap(proceso1,comienzaHueco1);
    listarBitMap();
     */

    //Conexion

    manejarConexionesConUMC();

    	free(bitMap);
		return 0;
}

 //----------Funciones para lista de procesos en swap

void agregarProcesoAListaSwap(proceso* procesoAInsertar){
	proceso* auxiliar1 = listaSwap;
	proceso* auxiliar2 = NULL;
	while(auxiliar1 != NULL && (auxiliar1->comienzo < procesoAInsertar->comienzo)){
		auxiliar2 = auxiliar1;
		auxiliar1 = auxiliar2->procesoSiguiente;
	}
	if(listaSwap == NULL){
		listaSwap = procesoAInsertar;
		procesoAInsertar->procesoSiguiente = NULL;
	} else {

		if(procesoAInsertar->comienzo == 0){

			procesoAInsertar->procesoSiguiente = listaSwap;
			listaSwap = procesoAInsertar;

		} else {

	auxiliar2->procesoSiguiente = procesoAInsertar;
	procesoAInsertar->procesoSiguiente = auxiliar1;

		}
	}

}
 //----------Funciones para crear procesos y manejarlos

	proceso crearProceso(int pid, int cantidadDePaginas){
		proceso proceso;
		proceso.pid = pid;
		proceso.cantidadDePaginas = cantidadDePaginas;

		return proceso;
	}


	void iniciar(void){
		int pid;
		int cantidadPaginas;
		recv(socketAdministradorDeMemoria, &pid, sizeof(int), 0);
		recv(socketAdministradorDeMemoria, &cantidadPaginas, sizeof(int), 0);
		proceso proceso = crearProceso(pid, cantidadPaginas);
		if(entraProceso(proceso)){
			//El proceso entra, realizar insercion
			pthread_mutex_lock(&mutex);
			insertarProceso(&proceso);
			pthread_mutex_unlock(&mutex);
		} else {
			//El proceso no entra, avisar rechazo
	}
}

	int entraProceso(proceso proceso){

			int paginasLibres;
			int pag = 0;
			for ( ; pag < (swap_configuracion.CANTIDAD_PAGINAS); pag++) {
				if(bitMap[pag]==0) paginasLibres++;
			}

			if(paginasLibres >= proceso.cantidadDePaginas){
				return 1;
			}else{
				return 0;
			}
		}



	void insertarProceso(proceso* proceso){
		int comienzoDelHueco;
		comienzoDelHueco = hayHuecoDondeCabeProceso(proceso);
		if(comienzoDelHueco >= 0){
			proceso->comienzo = comienzoDelHueco;
			actualizarBitMap(proceso,comienzoDelHueco);
			agregarProcesoAListaSwap(proceso);
		} else {
			//compactar();
		}

	}

	//Se fija si hay un hueco, si lo hay devuelve donde comienza
	int hayHuecoDondeCabeProceso(proceso* proceso){

		int paginaActual = 0;
		int ultimaPagina = swap_configuracion.CANTIDAD_PAGINAS;
		int paginasLibresConsecutivas = 0;

		while(paginaActual <= ultimaPagina){

			if(bitMap[paginaActual] == 0){
				paginasLibresConsecutivas++;
				paginaActual++;

			if(proceso->cantidadDePaginas == paginasLibresConsecutivas){
				return (paginaActual - proceso->cantidadDePaginas); } else { }

			} else {
				paginasLibresConsecutivas = 0;
				paginaActual++;
			}

		}

	}

	void compactar(void){
		dormir();
		proceso* auxiliar1 = listaSwap;
		proceso* auxiliar2 = listaSwap;
		if(auxiliar1->comienzo == 0){
			auxiliar1 = auxiliar2->procesoSiguiente;
			while(auxiliar1 != NULL){
			if ((auxiliar2->comienzo + auxiliar2->cantidadDePaginas) + 1
					== auxiliar1->comienzo){
				auxiliar2 = auxiliar1;
				auxiliar1 = auxiliar2->procesoSiguiente;
				} else {
				unirProcesos(auxiliar2,auxiliar1);
				auxiliar2 = auxiliar1;
				auxiliar1 = auxiliar2->procesoSiguiente;
				}
			}

		} else {
			//moverPrimerProceso(); //Moveria el primer proceso a que tenga comienzo en pagina 0
			//Aqui vendria un copy paste de lo que esta arriba
		}

	}

	void unirProcesos(proceso* procesoAnterior,proceso* procesoAJuntar){
		int nuevoComienzo = procesoAnterior->comienzo + 1;
		moverPaginas(procesoAJuntar, nuevoComienzo);
		eliminarDelBitMapLasPaginasDelProceso(procesoAJuntar);
		procesoAJuntar->comienzo = nuevoComienzo;
		actualizarBitMap(procesoAJuntar,nuevoComienzo);

	}

	void moverPaginas(proceso* procesoAJuntar,int nuevoComienzo){
			FILE* archivo;
			archivo = fopen(swap_configuracion.NOMBRE_SWAP,"r+");
			int tamanioPagina = swap_configuracion.TAMANIO_PAGINA;
			fseek(archivo,tamanioPagina*(procesoAJuntar->comienzo),SEEK_SET);
			char* texto = malloc(tamanioPagina* (procesoAJuntar->cantidadDePaginas));
			fread(texto,tamanioPagina,procesoAJuntar->cantidadDePaginas,archivo);
			fseek(archivo,tamanioPagina*nuevoComienzo,SEEK_SET);
			fwrite(texto,sizeof(char),sizeof(texto), archivo );
			fclose(archivo);
			free(texto);

	}

	void dormir(){
		usleep(swap_configuracion.RETARDO_COMPACTACION);
	}

	//--Funciones del BitMap

	void inicializarBitMap(){
			int pag = 0;
			int cantidadPaginas = swap_configuracion.CANTIDAD_PAGINAS;
			bitMap = (int *)malloc (cantidadPaginas*sizeof(int));
			for(; pag <= cantidadPaginas; pag++) {
						bitMap[pag] = 0;
						//printf("Pagina %d: %d \n",pag,bitMap[pag]);
					}
		}

	void eliminarDelBitMapLasPaginasDelProceso(proceso* proceso){
		int paginasActualizadas = 0;
		int comienzo = proceso->comienzo;
				while(paginasActualizadas < proceso->cantidadDePaginas){
					bitMap[comienzo] = 0;
					comienzo++;
					paginasActualizadas++;
				}

	}

	void actualizarBitMap(proceso* proceso,int comienzoDelHueco){
		int paginasActualizadas = 0;
		while(paginasActualizadas < proceso->cantidadDePaginas){
			bitMap[comienzoDelHueco] = 1;
			comienzoDelHueco++;
			paginasActualizadas++;
		}
		listarBitMap();
	}

	void listarBitMap(){
		int pag = 0;
		int cantidadPaginas = swap_configuracion.CANTIDAD_PAGINAS;
		for(; pag < cantidadPaginas; pag++) {

				printf("Pagina %d: %d \n",pag,bitMap[pag]);
		}
	}


 //---------Funciones para crear el archivo y manejarlo

void crearArchivo(){
	char* datos = malloc(100);
	sprintf(datos,"dd if=/dev/zero of=%s bs=%d count=%d",swap_configuracion.NOMBRE_SWAP,swap_configuracion.TAMANIO_PAGINA,swap_configuracion.CANTIDAD_PAGINAS);
	system(datos);
	free(datos);
	printf("El archivo se crea correctamente.\n");
}

void inicializarArchivo(){
	FILE* archivo = fopen(swap_configuracion.NOMBRE_SWAP,"r+");
		fseek(archivo,0,SEEK_END);
		char* texto = malloc(swap_configuracion.CANTIDAD_PAGINAS*swap_configuracion.TAMANIO_PAGINA);
		memset(texto,'\0',swap_configuracion.CANTIDAD_PAGINAS*swap_configuracion.TAMANIO_PAGINA);
		fwrite(texto,sizeof(char),sizeof(texto), archivo );
		free(texto);
		fclose(archivo);

}
 //---------Funcion para extraer los datos del archivo de configuracion

t_reg_config get_config_params(void){

	t_config * swap_config = NULL;
	char * swap_config_path = "swap_config.cfg";
	t_reg_config reg_config;

	swap_config = config_create(swap_config_path);

	// 1 get PUERTO_ESCUCHA          --------------
	if (config_has_property(swap_config,"PUERTO_ESCUCHA")){
		reg_config.PUERTO_ESCUCHA = config_get_int_value(swap_config,"PUERTO_ESCUCHA");
		printf("PUERTO_ESCUCHA= %d \n", reg_config.PUERTO_ESCUCHA);

	}
	else{
			printf("No se encontro PUERTO_ESCUCHA \n");
	}

	// 2 get PUERTO_ESCUCHA          --------------
		if (config_has_property(swap_config,"NOMBRE_SWAP")){
			reg_config.NOMBRE_SWAP = config_get_string_value(swap_config,"NOMBRE_SWAP");
			printf("NOMBRE_SWAP= %s \n", reg_config.NOMBRE_SWAP);

		}
		else{
				printf("No se encontro PUERTO_ESCUCHA \n");
		}

	// 3 get CANTIDAD_PAGINAS
	if (config_has_property(swap_config,"CANTIDAD_PAGINAS")){
		reg_config.CANTIDAD_PAGINAS = config_get_int_value(swap_config,"CANTIDAD_PAGINAS");
		printf("CANTIDAD_PAGINAS= %d \n", reg_config.CANTIDAD_PAGINAS);

	}
	else{
			printf("No se encontro CANTIDAD_PAGINAS \n");
	}

	// 4 get TAMAÑO_PAGINA
	if (config_has_property(swap_config,"TAMANIO_PAGINA")){
		reg_config.TAMANIO_PAGINA = config_get_int_value(swap_config,"TAMANIO_PAGINA");
		printf("TAMANIO_PAGINA= %d \n", reg_config.TAMANIO_PAGINA);

	}
	else{
			printf("No se encontro TAMANIO_PAGINA \n");
	}

	// 5 get RETARDO_COMPACTACION
	if (config_has_property(swap_config,"RETARDO_COMPACTACION")){
		reg_config.RETARDO_COMPACTACION = config_get_int_value(swap_config,"RETARDO_COMPACTACION");
		printf("RETARDO_COMPACTACION= %d \n", reg_config.RETARDO_COMPACTACION);

	}
	else{
			printf("No se encontro RETARDO_COMPACTACION \n");
	}

	return reg_config;

	config_destroy(swap_config);

}

	//Conexiones

	int conectarseConUMC(struct server servidor){
		struct sockaddr_in direccionCliente;
				unsigned int tamanioDireccion = sizeof(struct sockaddr);
				int cliente = accept(servidor.socketServer, (struct sockaddr*) &direccionCliente, &tamanioDireccion);
				if(cliente < 0){

					perror("Falló el accept.");
					printf("No se conecto.\n");

				} else {

					printf("Se conecto con la UMC.\n");
					return cliente;

				}
	}

			void manejarConexionesConUMC(void){
				int status = 1;
				struct server servidor;
				servidor = crearServer(swap_configuracion.PUERTO_ESCUCHA);
				ponerServerEscucha(servidor);
				printf("Escuchando UMC en socket %d \n", servidor.socketServer);
				socketAdministradorDeMemoria = conectarseConUMC(servidor);

				    	while(status){
				    		status = recibirMensajes();
				    	}

				    	printf("Desconectado.\n");

			}

			int recibirMensajes(){

				int header = recibirHeader();

					switch(header){

					        case '60':
								iniciar();
								return 1;

							/*case '':
								finalizar();
								return 1;
							case '':
								leer();
								return 1;
							case '':
								escribir();
								return 1;
							break;
							}*/
					return 0;
			}
	}

	int recibirHeader(void){
		int header;
		recv(socketAdministradorDeMemoria, &header, sizeof(header), 0);
		return header;
	}

