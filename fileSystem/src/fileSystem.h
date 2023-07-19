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
#include <commons/collections/list.h>
#include <commons/log.h>
#include <commons/bitarray.h>
#include <pthread.h>
#include <utils.h>
#include <sockets.h>
#include <compartido.h>
#include <serializacion.h>
#include <utils.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <assert.h>
#include <dirent.h>
#include <errno.h>
//------------------ FUNC-DEFINIDAS ------------------//
#define ARRAY_SIZE(arr) (sizeof(arr)/sizeof((arr)[0]))
//----------------------------------------------------//

#define FS_LOG "filesySystem.log"
#define FS_CONFIG "fileSystem.config"
#define FS_NAME "File_System"
#define PATH "/home/utnso/fs" //TODO: cambiar las direcciones (/home/utnso/tp-2023-1c-EstaEsLaVencida/files)
#define PATH_FCB "/home/utnso/fs/FCB" //TODO: cambiar las direcciones (/home/utnso/tp-2023-1c-EstaEsLaVencida/files/FCB)
#define TAMANIO_DE_PUNTERO 4

const char *contenido_superbloque[] = {
	"BLOCK_SIZE=64",
	"BLOCK_COUNT=65536"
};

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

typedef struct{
   int block_size;
   int block_count;
}datos_superbloque;

// ------------------------ESTRUCTURAS------------------------ //
typedef struct{
	FILE* archivo;
	char* nombre_archivo; 
	int tamanio_archivo; //expresado en Bytes
	uint32_t puntero_directo; //Lista de uint32_t
	uint32_t puntero_indirecto; //Lista de uint32_t
}t_fcb; 

typedef struct{
	int fid; //Identificador del bloque
	void* dato;
	int pos;
}t_block;

void* datos_memoria;

// ------------------------ARCHIVOS------------------------ //

FILE* F_BITMAP;
FILE* F_BLOCKS;
FILE* F_FCB;

// ------------------------VARIABLES GLOBALES------------------------ //
t_bitarray* bitmap;
size_t TAM_BITMAP;

t_list* lista_fcb;

t_list* lista_bloques;

// ------------------------CONEXIONES------------------------ //
t_log* logger;

t_config* config;
t_config* config_superbloque;
//TODO: Ver si lo sacamos o dejamos
t_config* config_bitmap; 
t_config* config_bloques;
t_config* config_fcb;
datos_config datos;
datos_superbloque superbloque;
int server_fd;
int conexion_memoria;
pthread_t hilo_conexion_kernel;
pthread_t hilo_conexion_memoria;

// ------------------------FUNCIONES------------------------ //
void* atender_kernel(void);
void* atender_memoria(void);
void enviar_respuesta_kernel(op_code);


char* abrir_archivo_fcb(char*, char*);
void crear_archivo(const char*, const char**, int);
void crear_archivo_fcb(char*);
t_fcb* crear_fcb(t_pcb* pcb);
void crear_archivo_bloques();
FILE* abrir_archivo(char*, char*);

int buscar_fcb(char*);
int buscar_archivo_fcb(char*);
t_fcb* obtener_fcb(t_pcb*);
FILE* obtener_archivo(char*);
void actualizar_lista_fcb(t_fcb*);
void agrandar_archivo(t_fcb*, int);
void achicar_archivo(t_fcb*, int);
char* concatenar_path(char*);
int convertir_byte_a_bit(int);

void escribir_bitmap(t_list*, int);
void leer_bitmap();
void leer_bitarray(t_bitarray*);
void crear_bitmap();
void crear_archivo_bitmap();
void cerrar_bitmap();

void crear_lista_bloques();
void liberar_lista_bloques();


void finalizar_fs();
void inicializar_superbloque();
void inicializar_config();
void iniciar_estructura_fs(const char *contenidos[]);
t_config* iniciar_config_fs(char*);
void iterator(char*);
void reemplazar_y_concatenar_palabra(char*, const char*, const char*, const char*);
void set_tamanio_archivo(FILE*, int);

#endif /* FILESYSTEM_H_ */
