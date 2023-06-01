/*
 ============================================================================
 Name        : fileSystem.c
 Author      : EstaEsLaVencida
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include "fileSystem.h"

int main(void) {
	logger = iniciar_logger(FS_LOG, FS_NAME);;
	config = iniciar_config(FS_CONFIG);

	inicializar_config();
	iniciar_estructura_fs();

	log_info(logger,"PATH_SUPERBLOQUE: %s", datos.path_superbloque);
	log_info(logger,"PATH_BITMAP: %s", datos.path_bitmap);
	log_info(logger,"PATH_BLOQUES: %s", datos.path_bloques);
	log_info(logger,"PATH_FCB: %s", datos.path_fcb);

	int block_size = config_get_int_value(config_superbloque, "BLOCK_SIZE");
	int block_count = config_get_int_value(config_superbloque, "BLOCK_COUNT");

	log_info(logger,"block_size: %i \tblock_count: %i", block_size, block_count);

	pthread_create(&hilo_conexion_kernel,NULL,(void*) atender_kernel,NULL);
	pthread_create(&hilo_conexion_memoria,NULL,(void*) atender_memoria,NULL);

	pthread_join(hilo_conexion_kernel,NULL);
	pthread_join(hilo_conexion_memoria,NULL);

	finalizar_fs();
	return EXIT_SUCCESS;
}

void inicializar_config(){
	datos.ip_memoria = config_get_string_value(config,"IP_MEMORIA");
	datos.puerto_memoria = config_get_string_value(config,"PUERTO_MEMORIA");
	datos.puerto_escucha = config_get_string_value(config,"PUERTO_ESCUCHA");
	datos.path_superbloque = config_get_string_value(config,"PATH_SUPERBLOQUE");
	datos.path_bitmap = config_get_string_value(config,"PATH_BITMAP");
	datos.path_bloques = config_get_string_value(config,"PATH_BLOQUES");
	datos.path_fcb = config_get_string_value(config,"PATH_FCB");
	datos.ret_acceso_bloque = config_get_int_value(config,"RETARDO_ACCESO_BLOQUE");

	reemplazarYConcatenarPalabra(datos.path_superbloque, "~/fs", "/home/utnso/fs/", "superbloque.dat");
	reemplazarYConcatenarPalabra(datos.path_bitmap, "~/fs", "/home/utnso/fs/", "bitmap.dat");
	reemplazarYConcatenarPalabra(datos.path_bloques, "~/fs", "/home/utnso/fs/", "bloques.dat");
	reemplazarYConcatenarPalabra(datos.path_fcb, "~/fs", "/home/utnso/fs/", "fcb");

}

void iniciar_estructura_fs(){
	config_superbloque = iniciar_config_fs(datos.path_superbloque);
	// config_bitmap = iniciar_config_fs(datos.path_bitmap);
	// config_bloques = iniciar_config_fs(datos.path_bloques);
	// config_fcb = iniciar_config_fs(datos.path_fcb);
	int valor = mkdir(PATH, 0777); //TODO: preguntar si existe la carpeta o si lo tenemos q crear
	if(valor < 0){
		log_warning(logger, "Ya existe el directo(%s)", PATH);
	}
	//Si existe la carpeta crea los archivos
	bitmap = fopen(datos.path_bitmap,"a");
	bloques = fopen(datos.path_bloques,"a");
	fcb = fopen(datos.path_fcb,"a");
}

t_config* iniciar_config_fs(char * path) {
	t_config* config = iniciar_config(path);
	
	if(config == NULL) {
		log_error(logger, "No se pudo leer el path (%s)", path);
	}

	return config;
}

FILE* crear_estructuras(char* path){
	FILE* file =fopen(path,"a");
	return file; 
}

void finalizar_fs(){
	log_destroy(logger);
	config_destroy(config);
	close(server_fd);
	close(conexion_memoria);
	fclose(bitmap);
	fclose(bloques);
	fclose(fcb);
}

void* atender_kernel(void){
	server_fd = iniciar_servidor(logger,"127.0.0.1",datos.puerto_escucha);
	log_info(logger, "Fyle System listo para recibir al kernel");
	int *cliente_fd = esperar_cliente(logger,server_fd);
	while(1){
		t_list* lista;
		while(1){
			int cod_op = recibir_operacion(*cliente_fd);
			switch (cod_op) {
				case MENSAJE:
					recibir_mensaje(logger,*cliente_fd);
					break;
				case PAQUETE:
					lista = recibir_paquete(*cliente_fd);
					log_info(logger, "Me llegaron los siguientes valores:\n");
					list_iterate(lista, (void*) iterator);
					break;
				case -1:
					log_error(logger, "el cliente se desconecto. Terminando servidor");
					log_destroy(logger);
					config_destroy(config);
					close(*cliente_fd);
					close(server_fd);
					//return EXIT_FAILURE;
					exit(1);
				default:
					log_warning(logger,"Operacion desconocida. No quieras meter la pata");
					break;
			}
		}
	}
}

void* atender_memoria(void){
	conexion_memoria = crear_conexion(datos.ip_memoria,datos.puerto_memoria);
	enviar_mensaje("Hola te saludo desde el Fyle System",conexion_memoria);
	while(1){
		t_list* lista;
		while(1){
			int cod_op = recibir_operacion(conexion_memoria);
			switch (cod_op) {
				case MENSAJE:
					recibir_mensaje(logger,conexion_memoria);
					break;
					case PAQUETE:
					lista = recibir_paquete(conexion_memoria);
					log_info(logger, "Me llegaron los siguientes valores:\n");
					list_iterate(lista, (void*) iterator);
					break;
				case -1:
					log_error(logger, "el cliente se desconecto. Terminando servidor");
					log_destroy(logger);
					config_destroy(config);
					close(conexion_memoria);
					//return EXIT_FAILURE;
					exit(1);
				default:
					log_warning(logger,"Operacion desconocida. No quieras meter la pata");
					break;
			}
		}
	}
}

void iterator(char* value) {
	log_info(logger,"%s", value);
}

void reemplazarYConcatenarPalabra(char *texto, const char *palabraBuscada, const char *palabraReemplazo, const char *palabraConcatenar) {
    char *posicion;
    int longitudPalabraConcatenar = strlen(palabraConcatenar);

    // Buscar la primera aparición de la palabra buscada
    posicion = strstr(texto, palabraBuscada);

    while (posicion != NULL) {
        // Copiar la palabra de reemplazo en lugar de la palabra buscada
        strcpy(posicion, palabraReemplazo);

        // Concatenar la palabra adicional después de la palabra de reemplazo
        strcat(posicion, palabraConcatenar);

        // Buscar la siguiente aparición de la palabra buscada
        posicion = strstr(posicion + strlen(palabraReemplazo) + longitudPalabraConcatenar, palabraBuscada);
    }
}