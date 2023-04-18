/*
 ============================================================================
 Name        : cpu.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include "cpu.h"

t_log* logger;
t_config* config;
datos_config datos;
int server_fd;
int conexion_memoria;
pthread_t hilo_conexion_kernel;
pthread_t hilo_conexion_memoria;
//t_list* deserializar_lista_instrucciones(t_buffer* buffer);
//t_pcb* deserializar_pcb(t_buffer* buffer);
//t_buffer* desempaquetar(t_paquete* paquete, int cliente_fd);
void mostrar(t_instruccion* inst);
void mostrar_parametro(char* value);

int main(void) {
	logger = iniciar_logger("cpu.log","CPU");;
	config = iniciar_config("cpu.config");

	datos.ip_memoria = config_get_string_value(config,"IP_MEMORIA");
	datos.puerto_memoria = config_get_string_value(config,"PUERTO_MEMORIA");
	datos.puerto_escucha = config_get_string_value(config,"PUERTO_ESCUCHA");
	datos.ret_instruccion = config_get_int_value(config,"RETARDO_INSTRUCCION");
	datos.tam_max_maximo = config_get_int_value(config,"TAM_MAX_SEGMENTO");
	pthread_create(&hilo_conexion_kernel,NULL,(void*) atender_kernel,NULL);
	pthread_create(&hilo_conexion_memoria,NULL,(void*) atender_memoria,NULL);

	pthread_join(hilo_conexion_kernel,NULL);
	pthread_join(hilo_conexion_memoria,NULL);
	log_destroy(logger);
	config_destroy(config);
	close(server_fd);
	close(conexion_memoria);
	return EXIT_SUCCESS;
}

void* atender_kernel(void){
	server_fd = iniciar_servidor(logger,"127.0.0.1",datos.puerto_escucha);
	log_info(logger, "CPU listo para recibir al kernel");
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->buffer = malloc(sizeof(t_buffer));
	t_buffer* buffer = malloc(sizeof(t_buffer));
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

				case EJECUTAR:
					log_info(logger,"Paso por ejecutar");
					buffer = desempaquetar(paquete,*cliente_fd);
					t_pcb* pcb = deserializar_pcb(buffer);
					log_info(logger,"El PID ES %i",pcb->pid);
					list_iterate(pcb->lista_instrucciones, (void*) mostrar);
					//ejecutar_pcb(pcb);
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
	enviar_mensaje("Hola te saludo desde el cpu",conexion_memoria);
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

void mostrar(t_instruccion* inst){
	log_info(logger,"La instruccion es %i",inst->nombre);
	list_iterate(inst->parametros, (void*) mostrar_parametro);
}

void mostrar_parametro(char* value){
	log_info(logger,"Parametro %s",value);
}
