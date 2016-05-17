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

//Variables globales

t_reg_config swap_configuracion;

int socketAdministradorDeMemoria;

int *bitMap;

t_list* listaDeProcesosEnSwap;




int main(void) {


	//Leo el archivo de configuración
	swap_configuracion = get_config_params();

	inicializarBitMap();

	//Creo el archivo de log
	t_log* log_swap = log_create("log_swap", "Swap", false, LOG_LEVEL_INFO);

	//Archivo swap
    crearArchivo();
    inicializarArchivo();

    //Conexion

    manejarConexionesConUMC();

    	free(bitMap);
		return 0;
}

 //----------Funciones para crear procesos y manejarlos

	proceso crearProceso(int pid, int cantidadDePaginas){
		proceso proceso;
		proceso.pid = pid;
		proceso.cantidadDePaginas = cantidadDePaginas;

		return proceso;
	}

	int entraProceso(proceso proceso){

		int paginasLibres;
		int pag = 0;
		for ( ; pag < (swap_configuracion.CANTIDAD_PAGINAS); pag++) {
			if(bitMap[pag]==0) paginasLibres++;
		}

		if(paginasLibres > proceso.cantidadDePaginas){
			return 1;
		}else{
			return 0;
		}
	}

	void inicializarBitMap(){
		int pag = 0;
		int cantidadPaginas = swap_configuracion.CANTIDAD_PAGINAS;
		bitMap = (int *)malloc (cantidadPaginas*sizeof(int));
		for(; pag <= cantidadPaginas; pag++) {
					bitMap[pag] = 0;
					printf("Pagina %d: %d \n",pag,bitMap[pag]);
				}
	}

	void iniciar(void){
		int pid;
		int cantidadPaginas;
		recv(socketAdministradorDeMemoria, &pid, sizeof(int), 0);
		recv(socketAdministradorDeMemoria, &cantidadPaginas, sizeof(int), 0);
		proceso proceso = crearProceso(pid, cantidadPaginas);
		if(entraProceso(proceso)){
			//El proceso entra, realizar insercion
		} else {
			//El proceso no entra, avisar rechazo
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
