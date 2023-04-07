/*
 ============================================================================
 Name        : kernel.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include "kernel.h"

t_log* logger;
t_config* config;
datos_config datos;
int server_fd;
int* cliente_fd;
int conexion_cpu;
int conexion_memoria;
int conexion_filesystem;
int main(void) {
	pthread_t hilo_atender_consolas;

	logger = iniciar_logger("kernel.log","Kernel");;
	config = iniciar_config("kernel.config");
	datos.ip_memoria = config_get_string_value(config,"IP_MEMORIA");
	datos.puerto_memoria = config_get_string_value(config,"PUERTO_MEMORIA");
	datos.ip_filesystem = config_get_string_value(config,"IP_FILESYSTEM");
	datos.puerto_filesystem = config_get_string_value(config,"PUERTO_FILESYSTEM");
	datos.ip_cpu = config_get_string_value(config,"IP_CPU");
	datos.puerto_cpu = config_get_string_value(config,"PUERTO_CPU");
	datos.puerto_escucha = config_get_string_value(config,"PUERTO_ESCUCHA");

	conexion_cpu = crear_conexion(datos.ip_cpu,datos.puerto_cpu);
	conexion_memoria = crear_conexion(datos.ip_memoria,datos.puerto_memoria);
	conexion_filesystem = crear_conexion(datos.ip_filesystem,datos.puerto_filesystem);

	enviar_mensaje("Hola te estoy hablando desde el kernel",conexion_cpu);
	enviar_mensaje("Hola te estoy hablando desde el kernel",conexion_memoria);
	enviar_mensaje("Hola te estoy hablando desde el kernel",conexion_filesystem);

	server_fd = iniciar_servidor(logger,"127.0.0.1",datos.puerto_escucha);
	log_info(logger, "KERNEL listo para recibir a las consolas");

	cliente_fd = malloc(sizeof(int));//

	while(1){
		cliente_fd = esperar_cliente(logger,server_fd);
		pthread_create(&hilo_atender_consolas,NULL,(void*) atender_consolas,cliente_fd);
		pthread_detach(hilo_atender_consolas);
	}

	log_destroy(logger);
	config_destroy(config);
	close(server_fd);

}

void atender_consolas(void* data){
	int cliente_fd = *((int*)data);
	t_list* lista;
	while(1){

		int cod_op = recibir_operacion(cliente_fd);
		switch (cod_op) {
			case MENSAJE:
				recibir_mensaje(logger,cliente_fd);
				break;
			case PAQUETE:
				lista = recibir_paquete(cliente_fd);
				log_info(logger, "Me llegaron los siguientes valores:\n");
				list_iterate(lista, (void*) iterator);
				break;
			case -1:
				log_error(logger, "el cliente se desconecto. Terminando servidor");
				log_destroy(logger);
				config_destroy(config);
				close(cliente_fd);
				close(server_fd);
				free(data);
				//return EXIT_FAILURE;
				exit(1);
			default:
				log_warning(logger,"Operacion desconocida. No quieras meter la pata");
				break;
		}
	}

}

void iterator(char* value) {
	log_info(logger,"%s", value);
}
