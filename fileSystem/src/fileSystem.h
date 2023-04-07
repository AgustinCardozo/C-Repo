/*
 * fileSystem.h
 *
 *  Created on: Apr 7, 2023
 *      Author: utnso
 */

#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_

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
	char* puerto_escucha;
	char* path_superbloque;
	char* path_bitmap;
	char* path_bloques;
	char* path_fcb;
	int ret_acceso_bloque;
}datos_config;

void* atender_kernel(void);
void* atender_memoria(void);
void iterator(char* value);

#endif /* FILESYSTEM_H_ */
