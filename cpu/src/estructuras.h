/*
 * estructuras.h
 *
 *  Created on: 16/5/2016
 *      Author: utnso
 */

#ifndef ESTRUCTURAS_H_
#define ESTRUCTURAS_H_

typedef struct {
	int pagina;
	int offset;
	int tamanio;
}__attribute__((packed))
solcUMC;

typedef struct {
	int pagina;
	int offset;
	int tamanio;
	int valor;
}__attribute__((packed))
almUMC;

typedef struct{
	int id;
	int ip;
	int sp;

}pcb;




#endif /* ESTRUCTURAS_H_ */
