/*
 * swapUMC.h
 *
 *  Created on: 18/6/2016
 *      Author: utnso
 */

#ifndef SWAPUMC_H_
#define SWAPUMC_H_

void leer(void);
void escribir();
void iniciar(void);
void finalizar(void);
void loguear(char *stringAlogear);
void logIniciar(proceso* proceso);
void logFinalizar(proceso* proceso);
void logRechazar(proceso* proceso);
void logCompactacionIniciada();
void logCompactacionFinalizada();
void logLectura(int pagina, proceso proceso);
void logEscritura(int pagina, proceso proceso);

#endif /* SWAPUMC_H_ */
