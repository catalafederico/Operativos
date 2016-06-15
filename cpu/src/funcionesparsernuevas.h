/*
 * FuncionesParser.h
 *
 *  Created on: 17/4/2016
 *      Author: utnso
 */

#ifndef FUNCIONESPARSERNUEVAS_H_
#define FUNCIONESPARSERNUEVAS_H_

	#include <parser/parser.h>
	t_puntero vardef(t_nombre_variable var);
	t_puntero getvarpos(t_nombre_variable var);
	t_valor_variable derf(t_puntero puntero_var);
	void asignar(t_puntero puntero_var, t_valor_variable valor);
	t_valor_variable getglobalvar(t_nombre_compartida var);
	t_valor_variable setglobalvar(t_nombre_compartida var, t_valor_variable valor);
	t_puntero_instruccion goint(t_nombre_etiqueta etiqueta);
	void fcall(t_nombre_etiqueta etiqueta, t_puntero funcion);
	void retornar(t_valor_variable retorno);
	void imprimir(t_valor_variable var);
	void imptxt(char* aimpprimir);
	void ionotif(t_nombre_dispositivo ioname, int tiempo);
	void inicialzarParser(int socketMem,int socketNuc);
	void waitCPU(t_nombre_semaforo semf);
	void signalCPU(t_nombre_semaforo semf);
	void fin();
	void fcallNR(t_nombre_etiqueta nombre);

#endif /* FUNCIONESPARSERNUEVAS_H_ */
