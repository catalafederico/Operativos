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
#include <string.h>



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

proceso* crearProceso(int pid, int cantidadDePaginas);

void inicializarBitMap(void);

void listarBitMap(void);

void actualizarBitMap(proceso* proceso,int comienzoDelHueco);

void eliminarDelBitMapLasPaginasDelProceso(proceso* proceso);

void insertarProceso(proceso* proceso);

int hayHuecoDondeCabeProceso(proceso* proceso);

void agregarProcesoAListaSwap(proceso* proceso);

void compactar(void);

void moverAPrimeraPosicionProceso(void);

void unirProcesos(proceso* procesoAnterior, proceso* procesoAJuntar);

void moverPaginas(proceso* procesoAJuntar,int nuevoComienzo);

int procesoSeEncuentraEnSwap(int pid);

proceso obtenerProceso(int pid);

void escribirPagina(int paginaAEscribir,void* texto);

void* leerPagina(int pagina);

void finalizar(void);

void eliminarProceso(int pid);

void* recibirCadena(void);

void mandarCadena(void* cadena);

void dormir(void);

void testEscribirEnPaginas(void);

void testLeerPagina();

void loguear(char *stringAlogear);

void logIniciar(proceso proceso);

void logFinalizar(proceso proceso);

void logRechazar(proceso proceso);

void logCompactacionIniciada();


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

	//Archivo swap

    crearArchivo();
    //inicializarArchivo();

    //listaSwap = malloc(sizeof(proceso));
    listaSwap = NULL;

/*
    proceso proceso2 = crearProceso(1,2);
    insertarProceso(&proceso2);
    escribirPagina(0, "Hola soy la pagina del proceso 1");

    proceso proceso1 = crearProceso(2,2);
    insertarProceso(&proceso1);
    escribirPagina(2, "Hola soy la pagina del proceso 2!!");

    listarBitMap();

    char* texto = leerPagina(0);
    char* texto1 = leerPagina(2);
    printf("%s\n", texto);
    printf("%s\n", texto1);
    eliminarProceso(1);
    compactar();
    printf("%s\n", "Compactado");
    listarBitMap();
    char* texto3 = leerPagina(0);
    char* texto4 = leerPagina(2);
    printf("%s\n", texto3);
    //printf("%s\n", texto4);
*/
 /*   listarBitMap();
    proceso proceso123 = obtenerProceso(2);
    printf("Cantidad de paginas del proceso 2: %d \n",proceso123.cantidadDePaginas);
    while(listaSwap != NULL){
    	printf("Pid: %d \n Comienzo: %d\n", listaSwap->pid, listaSwap->comienzo);
    	listaSwap = listaSwap->procesoSiguiente;
    }

    testEscribirEnPaginas();
    testLeerPagina();

    listaSwap = listaSwap->procesoSiguiente;
    compactar();
    listarBitMap();

    int comienzaHueco1 = hayHuecoDondeCabeProceso(proceso1);
    printf("Hueco comienza en:%d \n",comienzaHueco1);
    actualizarBitMap(proceso1,comienzaHueco1);
    listarBitMap();

  */

    //Conexion

    manejarConexionesConUMC();

    	free(bitMap);
    	free(listaSwap);
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

	proceso* crearProceso(int pid, int cantidadDePaginas){
		proceso* proceso = malloc(sizeof(proceso));
		proceso->pid = pid;
		proceso->cantidadDePaginas = cantidadDePaginas;

		return proceso;
	}


	void iniciar(void){
		int pid;
		int cantidadPaginas;
		recv(socketAdministradorDeMemoria, &pid, sizeof(int), 0);
		recv(socketAdministradorDeMemoria, &cantidadPaginas, sizeof(int), 0);
		proceso* proceso = crearProceso(pid, cantidadPaginas);
		if(entraProceso(*proceso)){
			//El proceso entra, realizar insercion
			pthread_mutex_lock(&mutex);
			insertarProceso(proceso);
			pthread_mutex_unlock(&mutex);
			//logIniciar(*proceso);
			int exito = 6;
			send(socketAdministradorDeMemoria,&exito, sizeof(int), 0);
		} else {
			//El proceso no entra, avisar rechazo
			//logRechazar(*proceso);
			printf("No hay cantidad de paginas suficientes para alojar el proceso %d", proceso->pid);
			int fracaso = 7;
			send(socketAdministradorDeMemoria,&fracaso, sizeof(int), 0);
	}
}

	void leer(void){
		int pid;
		int numeroPagina;
		recv(socketAdministradorDeMemoria, &pid, sizeof(int), 0);
		recv(socketAdministradorDeMemoria, &numeroPagina, sizeof(int), 0);
		if(procesoSeEncuentraEnSwap(pid)){
			proceso proceso = obtenerProceso(pid);
			if(numeroPagina >= proceso.cantidadDePaginas){
				printf("La pagina %d no se encuentra en el proceso %d y no puede ser leida\n",numeroPagina,pid);
				int fracaso = 7;
				send(socketAdministradorDeMemoria,&fracaso, sizeof(int), 0);
			} else {
				int exito = 6;
				send(socketAdministradorDeMemoria,&exito, sizeof(int), 0);
				int paginaALeer = proceso.comienzo + numeroPagina;
				void* texto = leerPagina(paginaALeer);
				//printf("%s\n",texto);
				mandarCadena(texto);
				free(texto);
			}
		} else {
			printf("Proceso %d no se encuentra en Swap y no puede ser leido\n",pid);
			int fracaso = 7;
			send(socketAdministradorDeMemoria,&fracaso, sizeof(int), 0);
		}
	}

	void escribir(){
		int pid;
		int numeroPagina;
		recv(socketAdministradorDeMemoria, &pid, sizeof(int), 0);
		recv(socketAdministradorDeMemoria, &numeroPagina, sizeof(int), 0);
		void* texto = recibirStream(socketAdministradorDeMemoria,swap_configuracion.TAMANIO_PAGINA);
		if(procesoSeEncuentraEnSwap(pid)){
			proceso proceso = obtenerProceso(pid);
			if(numeroPagina >= proceso.cantidadDePaginas){
				printf("La pagina %d no se encuentra en el proceso %d y no puede ser escrita\n",numeroPagina,pid);
				int fracaso = 7;
				send(socketAdministradorDeMemoria,&fracaso, sizeof(int), 0);
			} else {
				int paginaAEscribir = proceso.comienzo + numeroPagina;
				escribirPagina(paginaAEscribir,texto);
				int exito = 6;
				send(socketAdministradorDeMemoria,&exito, sizeof(int), 0);

			}
		} else {
			printf("Proceso %d no se encuentra en Swap y no puede ser escrito\n",pid);
			int fracaso = 7;
			send(socketAdministradorDeMemoria,&fracaso, sizeof(int), 0);
		}
	}

	void finalizar(void){
		int pid;
		recv(socketAdministradorDeMemoria, &pid, sizeof(int), 0);
		if (procesoSeEncuentraEnSwap(pid)){
			proceso procesoAEliminar = obtenerProceso(pid);
			logFinalizar(procesoAEliminar);
			eliminarProceso(pid);
			//Avisar a la UMC que se borro el proceso con exito
			int exito = 6;
			send(socketAdministradorDeMemoria,&exito, sizeof(int), 0);
		} else{
			printf("El proceso %d a finalizar no se encuentra en Swap\n",pid);
			//Avisar a la UMC que se fracaso en el borrado del proceso
			int fracaso = 7;
			send(socketAdministradorDeMemoria,&fracaso, sizeof(int), 0);
		}
	}

	void eliminarProceso(int pid){
		proceso* auxiliar1 = listaSwap->procesoSiguiente;
		proceso* auxiliar2 = listaSwap;

		while(auxiliar1 != NULL){
			if(listaSwap->pid == pid){
				eliminarDelBitMapLasPaginasDelProceso(listaSwap);
				listaSwap = listaSwap->procesoSiguiente;
			} else {
				if(auxiliar1->pid == pid){
					auxiliar2->procesoSiguiente = auxiliar1->procesoSiguiente;
					eliminarDelBitMapLasPaginasDelProceso(auxiliar1);
				} else {
					auxiliar2 = auxiliar1;
					auxiliar1 = auxiliar2->procesoSiguiente;
				}

			}
		}
	}

	void escribirPagina(int paginaAEscribir,void* texto){
		int tamanioPagina = swap_configuracion.TAMANIO_PAGINA;
		/*if(strlen(texto)>tamanioPagina){
				printf("El texto es mas grande que el tamaño de pagina entonces se corta\n");
			}*/
			FILE* archivo;
			archivo = fopen(swap_configuracion.NOMBRE_SWAP,"rb+");
			//printf("%s\n",texto);
			//printf("%d\n",strlen(texto));
			/*char* texto2 = malloc(tamanioPagina);
			memset(texto2,' ',tamanioPagina);
			char* texto0 = string_new();
			string_append(&texto0,texto);
			string_append(&texto0,texto2);
			char* texto1 = string_substring_until(texto0,tamanioPagina);*/
			fseek(archivo,tamanioPagina*paginaAEscribir,SEEK_SET);
			fwrite(texto,sizeof(char),tamanioPagina, archivo );
			//fprintf(archivo,"%s",texto1);
			fclose(archivo);
			//free(texto2);
			free(texto);
	}

	void* leerPagina(int pagina){
			FILE* archivo;
			int tamanioPagina = swap_configuracion.TAMANIO_PAGINA;
			//printf("%d\n",pagina);
			archivo = fopen(swap_configuracion.NOMBRE_SWAP,"rb+");
			fseek(archivo,tamanioPagina*pagina,SEEK_SET);
			void* texto = malloc(tamanioPagina);
			//texto[tamanioPagina] = NULL;
			fread(texto,tamanioPagina,1,archivo);
			fclose(archivo);
			//printf("%s\n",texto);
			return texto;
		}



	int procesoSeEncuentraEnSwap(int pid){
		proceso* auxiliar = listaSwap;
		while(auxiliar != NULL){

			if(auxiliar->pid == pid){
				return 1;
			} else {
				auxiliar = auxiliar->procesoSiguiente;
			}

			return 0;
		}
	}

	proceso obtenerProceso(int pid){

		proceso* auxiliar = listaSwap;

		while(auxiliar != NULL){

			if(auxiliar->pid == pid){

				return *auxiliar;

			} else {

				auxiliar = auxiliar->procesoSiguiente;

			}
		}
	}

	int entraProceso(proceso proceso){

			int paginasLibres = 0;
			int pag = 0;
			do{
				if(bitMap[pag]==0)
					paginasLibres++;
				if(paginasLibres >= proceso.cantidadDePaginas){
					return 1;
				}
				pag++;
			}while(pag < (swap_configuracion.CANTIDAD_PAGINAS));
			return 0;
			/*for ( ; pag < (swap_configuracion.CANTIDAD_PAGINAS); pag++) {
				if(bitMap[pag]==0) paginasLibres++;
			}

			if(paginasLibres >= proceso.cantidadDePaginas){
				return 1;
			}else{
				return 0;
			}*/
		}



	void insertarProceso(proceso* proceso){
		int comienzoDelHueco;
		comienzoDelHueco = hayHuecoDondeCabeProceso(proceso);
		if(comienzoDelHueco >= 0){
			proceso->comienzo = comienzoDelHueco;
			actualizarBitMap(proceso,comienzoDelHueco);
			agregarProcesoAListaSwap(proceso);
		} else {
			logCompactacionIniciada();
			compactar();
			comienzoDelHueco = hayHuecoDondeCabeProceso(proceso);
			proceso->comienzo = comienzoDelHueco;
			actualizarBitMap(proceso,comienzoDelHueco);
			agregarProcesoAListaSwap(proceso);
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

		if(paginasLibresConsecutivas == 0) return -1;

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
			moverAPrimeraPosicionProceso(); //Mueve el proceso mas cercano a comienzo 0, a dicha posicion ( 0 )
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
		}

	}

	void moverAPrimeraPosicionProceso(void){
		proceso* procesoAMover = listaSwap;
		moverPaginas(procesoAMover, 0);
		eliminarDelBitMapLasPaginasDelProceso(procesoAMover);
		procesoAMover->comienzo = 0;
		actualizarBitMap(procesoAMover,0);
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
			archivo = fopen(swap_configuracion.NOMBRE_SWAP,"rb+");
			int tamanioPagina = swap_configuracion.TAMANIO_PAGINA;
			fseek(archivo,tamanioPagina*(procesoAJuntar->comienzo),SEEK_SET);
			void* texto = malloc(tamanioPagina* (procesoAJuntar->cantidadDePaginas));
			fread(texto,tamanioPagina,procesoAJuntar->cantidadDePaginas,archivo);
			fseek(archivo,tamanioPagina*nuevoComienzo,SEEK_SET);
			fwrite(texto,sizeof(char),tamanioPagina, archivo );
			//fprintf(archivo,"%s",texto);
			fclose(archivo);
			free(texto);

	}

	void dormir(){
		usleep(swap_configuracion.RETARDO_COMPACTACION*1000000);
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
		//listarBitMap();
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
		fseek(archivo,0,SEEK_SET);
		char* texto = malloc(swap_configuracion.CANTIDAD_PAGINAS*swap_configuracion.TAMANIO_PAGINA);
		memset(texto,' ',swap_configuracion.CANTIDAD_PAGINAS*swap_configuracion.TAMANIO_PAGINA);
		//fwrite(texto,sizeof(char),sizeof(texto), archivo );
		fprintf(archivo,"%s",texto);
		fclose(archivo);
		free(texto);

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
					loguear("No se pudo conectar con la UMC.");

				} else {
					loguear("Se conecto con la UMC.");
					printf("Se conecto con la UMC.\n");
					return cliente;

				}
	}

			void manejarConexionesConUMC(void){
				int status = 1;
				struct server servidor;
				servidor = crearServer(swap_configuracion.PUERTO_ESCUCHA);
				loguear("Se crea el socket correctamente.\0");
				ponerServerEscucha(servidor);
				printf("Escuchando UMC en socket %d \n", servidor.socketServer);
				loguear("Escuchando conexiones.\0");
				socketAdministradorDeMemoria = conectarseConUMC(servidor);

				    	while(status){
				    		status = recibirMensajes();
				    	}
				    	loguear("Se desconecto la UMC.");
				    	printf("Desconectado.\n");

			}

			int recibirMensajes(){

				int header = recibirHeader();

					switch(header){

					        case 50:
								iniciar();
								return 1;
							case 51:
								finalizar();
								return 1;
							case 52:
								leer();
								return 1;
							case 53:
								escribir();
								return 1;
							break;
							}
					return 0;
			}

	int recibirHeader(void){
		int header;
		recv(socketAdministradorDeMemoria, &header, sizeof(header), 0);
		return header;
	}

	void mandarCadena(void* cadena) {
		//int long_cadena = strlen(cadena)+1;
		//send(socketAdministradorDeMemoria, &long_cadena, sizeof(long_cadena), 0);
		send(socketAdministradorDeMemoria, cadena, swap_configuracion.TAMANIO_PAGINA, 0);
	}

	void* recibirCadena() {
		//int long_cadena =
		//recv(socketAdministradorDeMemoria, &long_cadena, sizeof(long_cadena), 0);
		void* cadena = recibirStream(socketAdministradorDeMemoria,swap_configuracion.TAMANIO_PAGINA);
		return cadena;
	}

	//--Log

	void loguear(char *stringAlogear){
		t_log* log_swap = log_create("log_swap", "Swap", false, LOG_LEVEL_INFO);
		log_info(log_swap, stringAlogear, "INFO");
		log_destroy(log_swap);
	}

	void logIniciar(proceso proceso){
		//char stringAlogear[200];
		//sprintf(stringAlogear,"Proceso asignado - PID: %d - Pagina inicial: %d - Paginas: %d - Tamaño: %d .",proceso.pid, proceso.comienzo,proceso.cantidadDePaginas,proceso.cantidadDePaginas*swap_configuracion.TAMANIO_PAGINA);
		loguear(("Proceso asignado - PID: %d - Pagina inicial: %d - Paginas: %d - Tamaño: %d .",proceso.pid, proceso.comienzo,proceso.cantidadDePaginas,proceso.cantidadDePaginas*swap_configuracion.TAMANIO_PAGINA));
	}

	void logFinalizar(proceso proceso){
		//char stringAlogear[200];
		//sprintf(stringAlogear,"Proceso liberado - PID: %d - Pagina inicial: %d - Paginas: %d - Tamaño: %d .",proceso.pid, proceso.comienzo,proceso.cantidadDePaginas,proceso.cantidadDePaginas*swap_configuracion.TAMANIO_PAGINA);
		loguear(("Proceso liberado - PID: %d - Pagina inicial: %d - Paginas: %d - Tamaño: %d .",proceso.pid, proceso.comienzo,proceso.cantidadDePaginas,proceso.cantidadDePaginas*swap_configuracion.TAMANIO_PAGINA));
	}

	void logRechazar(proceso proceso){
		//char stringAlogear[200];
		//sprintf(stringAlogear,"Proceso rechazado - PID: %d - Falta de espacio .",proceso.pid);
		loguear(("Proceso rechazado - PID: %d - Falta de espacio .",proceso.pid));
	}

	void logCompactacionIniciada(){
		//char stringAlogear[100];
		//sprintf(stringAlogear,"Compactacion iniciada");
		loguear("Compactacion iniciada");
	}

	//---Tests

	void testEscribirEnPaginas() {
		escribirPagina(2,"1234567890");
		escribirPagina(3,"dadadadada");
		escribirPagina(0,"12345678910111213");
		escribirPagina(1,"dsnaudsabifbasifasifhasufausbfasupfdhusafphsdauensayfgesyafo");
		escribirPagina(0,"hola                                                                                                                                                                                                                                                           F");
		escribirPagina(0,"12345678910111213");
		escribirPagina(1,"como andas");
	}

	void testLeerPagina() {
		char* texto = leerPagina(0);
		char* texto2 = leerPagina(1);
		char* texto3 = leerPagina(2);
		char* texto4 = leerPagina(3);
		printf("%s\n", texto);
		printf("%s\n", texto2);
		printf("%s\n", texto3);
		printf("%s\n", texto4);
	}
