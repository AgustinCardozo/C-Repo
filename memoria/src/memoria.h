/*
 * memoria.h
 *
 *  Created on: Apr 7, 2023
 *      Author: utnso
 */

#ifndef MEMORIA_H_
#define MEMORIA_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include <utils.h>
#include <sockets.h>
#include <compartido.h>

typedef struct{
	char* puerto_escucha;
	/*int tam_memoria;
	int tam_segmento;
	int cant_segmentos;
	int ret_memoria;
	int ret_compactacion;
	int algoritmo;*/
}datos_config;

void iterator(char* value);


#endif /* MEMORIA_H_ */
