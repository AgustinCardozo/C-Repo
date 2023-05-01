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

#define FS_LOG "filesySystem.log"
#define FS_CONFIG "fileSystem.config"
#define FS_NAME "File_System"
#define PATH "/home/utnso/fs"

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

FILE* bitmap;
FILE* bloques;
FILE* fcb;

t_log* logger;

t_config* config;
t_config* config_superbloque;
//TODO: Ver si lo sacamos o dejamos
t_config* config_bitmap; 
t_config* config_bloques;
t_config* config_fcb;
datos_config datos;
int server_fd;
int conexion_memoria;
pthread_t hilo_conexion_kernel;
pthread_t hilo_conexion_memoria;

void* atender_kernel(void);
void* atender_memoria(void);

void finalizar_fs();
void inicializar_config();
void iniciar_estructura_fs();
t_config* iniciar_config_fs(char*); 
void iterator(char*);

#endif /* FILESYSTEM_H_ */
