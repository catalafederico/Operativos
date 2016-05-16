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
}proceso;

// Funciones

t_reg_config get_config_params(void);

void crearArchivo();

void inicializarArchivo();

int conectarseConUMC(struct server servidor);

void manejarConexionesConUMC(void);

int recibirMensajes(void);

int recibirHeader(void);

void iniciar(void);

int entraProceso(proceso proceso,int bitMap[]);

proceso crearProceso(int pid, int cantidadDePaginas);

//Variables globales

t_reg_config swap_config;

int socketAdministradorDeMemoria;

int main(void) {

	//Variables locales

	int bitMap[swap_config.CANTIDAD_PAGINAS];

	//Creo el archivo de log
	t_log* log_swap = log_create("log_swap", "Swap", false, LOG_LEVEL_INFO);


	swap_config = get_config_params();

	//Archivo swap

    crearArchivo();
    inicializarArchivo();


    //Conexion

    manejarConexionesConUMC();



/*struct sockaddr_in direccionServidor;
		direccionServidor.sin_family = AF_INET;
		direccionServidor.sin_addr.s_addr = INADDR_ANY;
		direccionServidor.sin_port = htons(swap_config.PUERTO_ESCUCHA);

		int servidor = socket(AF_INET, SOCK_STREAM, 0);
		log_info(log_swap,"Socket creado correctamente.");

		//En teoria permite reutilizar el puerto (A veces falla)
		int activado = 1;
		setsockopt(servidor, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));

		if (bind(servidor, (void*) &direccionServidor, sizeof(direccionServidor)) != 0) {
			perror("Falló el bind");
			log_info(log_swap,"El socket no se pudo asociar al puerto.");
			return 1;
		}
		log_info(log_swap,"El socket se asocio correctamente al puerto.");

		listen(servidor, 10);
		printf("Estoy escuchando.\n");
		log_info(log_swap,"El socket esta escuchando conexiones.");

		struct sockaddr_in direccionCliente;
		unsigned int tamanioDireccion = sizeof(struct sockaddr);
		int cliente = accept(servidor, (struct sockaddr*) &direccionCliente, &tamanioDireccion);
		if(cliente < 0){
			perror("Falló el accept.");
			printf("No se conecto.\n");
			log_info(log_swap,"Falló el accept.");

		} else {

			printf("Se conecto.\n");
			log_info(log_swap,"El socket establecio una conexion correctamente.");

		}


		printf("Recibí una conexión en %d!!\n", cliente);

		char* buffer = malloc(1000);

			while (1) {
				int bytesRecibidos = recv(cliente, buffer, 1000, 0);
				if (bytesRecibidos <= 0) {
					perror("El cliente se desconectó.");
					return 1;
				}

				buffer[bytesRecibidos] = '\0';
				printf("Me llegaron %d bytes con %s\n", bytesRecibidos, buffer);
			}

			free(buffer);

		send(cliente, "Hola!", 6, 0);*/

		return 0;
}

 //----------Funciones para crear procesos y manejarlos

	proceso crearProceso(int pid, int cantidadDePaginas){
		proceso proceso;
		proceso.pid = pid;
		proceso.cantidadDePaginas = cantidadDePaginas;

		return proceso;
	}

	int entraProceso(proceso proceso,int bitMap[]){

		int paginasLibres;
		int pag = 0;
		for (pag ; pag < (swap_config.CANTIDAD_PAGINAS); ++pag) {
			if(bitMap[pag]==0) paginasLibres++;
		}

		if(paginasLibres > proceso.cantidadDePaginas){
			return 1;
		}else{
			return 0;
		}
	}

	void iniciar(void){

	}

 //---------Funciones para crear el archivo y manejarlo

void crearArchivo(){
	char* datos = malloc(100);
	sprintf(datos,"dd if=/dev/zero of=%s bs=%d count=%d",swap_config.NOMBRE_SWAP,swap_config.TAMANIO_PAGINA,swap_config.CANTIDAD_PAGINAS);
	system(datos);
	free(datos);
	printf("El archivo se crea correctamente.\n");
}

void inicializarArchivo(){
	FILE* archivo = fopen(swap_config.NOMBRE_SWAP,"r+");
		fseek(archivo,0,SEEK_END);
		char* texto = malloc(swap_config.CANTIDAD_PAGINAS*swap_config.TAMANIO_PAGINA);
		memset(texto,'\0',swap_config.CANTIDAD_PAGINAS*swap_config.TAMANIO_PAGINA);
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

	config_destroy(swap_config);
	return reg_config;
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
				servidor = crearServer(swap_config.PUERTO_ESCUCHA);
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
