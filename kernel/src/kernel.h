/*
 * kernel.h
 *
 *  Created on: Apr 7, 2023
 *      Author: utnso
 */

#ifndef KERNEL_H_
#define KERNEL_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include <pthread.h>
#include <utils.h>
#include <sockets.h>
#include <compartido.h>

typedef struct{
	char* ip_memoria;
	char* puerto_memoria;
	char* ip_filesystem;
	char* puerto_filesystem;
	char* ip_cpu;
	char* puerto_cpu;
	char* puerto_escucha;
	//char* algoritmo_planificacion;
	int est_inicial;
	float alfa;
	//int multiprogramacion;
	//char** recursos;
	//char** instancias;*/
}datos_config;

void iterator(char* value);
void atender_consolas(void* data);
#endif /* KERNEL_H_ */
