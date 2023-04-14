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
t_buffer* desempaquetar(t_paquete* paquete, int cliente_fd);
t_list* deserializar_lista_instrucciones(t_buffer* buffer);
int agregar_pid(t_buffer* buffer);
void mostrar(t_instruccion* inst);
void mostrar_parametro(char* value);

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
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->buffer = malloc(sizeof(t_buffer));
	t_buffer* buffer;
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

			case INSTRUCCIONES:
				buffer = desempaquetar(paquete,cliente_fd);
				t_list* lista_instrucciones = deserializar_lista_instrucciones(buffer);
				int pid = agregar_pid(buffer);
				log_info(logger,"EL PID QUE ME LLEGO ES %i",pid);
				//list_iterate(lista_instrucciones, (void*) mostrar);
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
				log_error(logger, "el cliente se desconecto. Terminando servidor");
				log_destroy(logger);
				config_destroy(config);
				close(cliente_fd);
				close(server_fd);
				free(data);
				//return EXIT_FAILURE;
				exit(1);
				break;
		}
	}

}

void iterator(char* value) {
	log_info(logger,"%s", value);
}

t_buffer* desempaquetar(t_paquete* paquete, int cliente_fd){
	recv(cliente_fd, &(paquete->buffer->size), sizeof(uint32_t), 0);
	paquete->buffer->stream = malloc(paquete->buffer->size);
	recv(cliente_fd, paquete->buffer->stream, paquete->buffer->size, 0);

	return paquete->buffer;
}

int agregar_pid(t_buffer* buffer){
	int id;
	int offset = buffer->size - 4;
	memcpy(&id, buffer->stream + offset, sizeof(int));
	offset += sizeof(int);
	return id;

}

t_list* deserializar_lista_instrucciones(t_buffer* buffer){

	t_list*	i_list;
	t_instruccion* instruccion;
	uint32_t i_count;
	uint32_t p_count;
	int offset;
	int	p_size;
	char* param;
	op_instruct	codigo;

	i_list = list_create();
	offset = 0;

	memcpy(&i_count, buffer->stream + offset, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	//log_info(logger,"Esta deserializando %i",i_count);

	for(int i=0; i < i_count; i++) {

			// codigo de la instruccion
			memcpy(&(codigo), buffer->stream + offset, sizeof(uint32_t));
			offset += sizeof(uint32_t);
			instruccion = malloc(sizeof(t_instruccion));
			instruccion->nombre = codigo;
			instruccion->parametros = list_create();

			// cantidad de parametros
			memcpy(&(p_count), buffer->stream + offset, sizeof(uint32_t));
			offset += sizeof(uint32_t);

			if (p_count > 0) {
				for (int p = 0; p < p_count; p++) {
					// longitud del parametro
					memcpy(&p_size, buffer->stream + offset, sizeof(uint32_t));
					offset += sizeof(uint32_t);

					// valor del parametro
					param = malloc(p_size);
					memcpy(param, buffer->stream + offset, p_size);

					list_add(instruccion->parametros, param);
					offset += p_size;
				}
			}

			list_add(i_list, instruccion);

		}




		return i_list;
}

void mostrar(t_instruccion* inst){
	log_info(logger,"La instruccion es %i",inst->nombre);
	list_iterate(inst->parametros, (void*) mostrar_parametro);
}

void mostrar_parametro(char* value){
	log_info(logger,"Parametro %s",value);
}
