/*
 * procesosUMC.h
 *
 *  Created on: 18/5/2016
 *      Author: Lucas Marino
 */

#ifndef PROCESOSUMC_H_
#define PROCESOSUMC_H_

void *procesos_UMC();
void conectarseConUmc(struct cliente clienteNucleo);
void notificarAUMCfpc(int id);

#endif /* PROCESOSUMC_H_ */
