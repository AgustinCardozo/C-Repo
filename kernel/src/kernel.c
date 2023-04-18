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
//t_buffer* desempaquetar(t_paquete* paquete, int cliente_fd);
int agregar_pid(t_buffer* buffer);
void mostrar(t_instruccion* inst);
void mostrar_parametro(char* value);
void mostrar_cola(t_queue*cola);
t_pcb* crear_pcb(t_buffer* buffer);
void enviar_pcb_a(t_pcb* pcb,int conexion, op_code codigo);
void* atender_cpu(void);

/////PLANIFICACION///////
//colas
t_cola*cola;
////TRANSICIONES////
void agregar_a_cola_new(t_pcb*pcb);
void agregar_a_cola_ready(t_pcb*pcb);
//void agregar_a_cola_hrrn(t_pcb*pcb);
void finalizar_proceso(t_pcb*pcb);
t_pcb*quitar_de_new_fifo();
algoritmo devolver_algoritmo(char*nombre);

pthread_t thread_atender_cpu;

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
	datos.algoritmo_planificacion=devolver_algoritmo(config_get_string_value(config,"ALGORITMO_PLANIFICACION"));
	datos.est_inicial = config_get_int_value(config,"ESTIMACION_INICIAL");
	datos.alfa = atof(config_get_string_value(config,"HRRN_ALFA"));
	datos.multiprogramacion=config_get_int_value(config,"GRADO_MAX_MULTIPROGRAMACION");

	conexion_cpu = crear_conexion(datos.ip_cpu,datos.puerto_cpu);
	conexion_memoria = crear_conexion(datos.ip_memoria,datos.puerto_memoria);
	conexion_filesystem = crear_conexion(datos.ip_filesystem,datos.puerto_filesystem);

	enviar_mensaje("Hola te estoy hablando desde el kernel",conexion_cpu);
	enviar_mensaje("Hola te estoy hablando desde el kernel",conexion_memoria);
	enviar_mensaje("Hola te estoy hablando desde el kernel",conexion_filesystem);

	pthread_create(&thread_atender_cpu,NULL,(void*) atender_cpu,NULL);

	server_fd = iniciar_servidor(logger,"127.0.0.1",datos.puerto_escucha);
	log_info(logger, "KERNEL listo para recibir a las consolas");

	cliente_fd = malloc(sizeof(int));//

	while(1){
		cliente_fd = esperar_cliente(logger,server_fd);
		pthread_create(&hilo_atender_consolas,NULL,(void*) atender_consolas,cliente_fd);
		pthread_detach(hilo_atender_consolas);
	}

	cola->cola_new_fifo=queue_create();
	cola->cola_ready_fifo=queue_create();
	cola->cola_ready_hrrn=queue_create();

	log_destroy(logger);
	config_destroy(config);
	close(server_fd);

}

void* atender_cpu(void){
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->buffer = malloc(sizeof(t_buffer));
	t_pcb* pcb;
	t_buffer* buffer;

	while(1){
		int cod_op = recibir_operacion(conexion_cpu);
			switch (cod_op) {
				case FINALIZAR:
					buffer = desempaquetar(paquete,conexion_cpu);
					pcb = deserializar_pcb(buffer);
					log_info(logger,"El pcb [%i] ha sido finalizado",pcb->pid);
					break;
				case -1:
					log_error(logger, "el cliente se desconecto. Terminando servidor");
					log_destroy(logger);
					config_destroy(config);
					close(conexion_cpu);
					close(server_fd);

					//return EXIT_FAILURE;
					exit(1);
				default:
					log_warning(logger,"Operacion desconocida. No quieras meter la pata");
					log_error(logger, "el cliente se desconecto. Terminando servidor");
					log_destroy(logger);
					config_destroy(config);
					close(conexion_cpu);
					close(server_fd);

					//return EXIT_FAILURE;
					exit(1);
					break;
			}
	}
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
				//t_list* lista_instrucciones = deserializar_lista_instrucciones(buffer);

				//int pid = agregar_pid(buffer);
				//t_pcb* pcb= crear_pcb(pid,lista_instrucciones);
				t_pcb* pcb= crear_pcb(buffer);
				/*if(pcb->pid>0){
					agregar_a_cola_new(pcb);
				}*/
				/*agregar_a_cola_ready(pcb);
				mostrar_cola(cola->cola_ready_fifo);
				mostrar_cola(cola->cola_ready_hrrn);
*/
				log_info(logger,"EL PID QUE ME LLEGO ES %i",pcb->pid);
				//mostrar_cola(cola->cola_new_fifo);


				list_iterate(pcb->lista_instrucciones, (void*) mostrar);

				enviar_pcb_a(pcb,conexion_cpu,EJECUTAR);


				log_info(logger,"El alfa es %d",pcb->estimacion);
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



int agregar_pid(t_buffer* buffer){
	int id;
	int offset = 0;
	memcpy(&id, buffer->stream + offset, sizeof(int));
	offset += sizeof(int);
	return id;

}

void mostrar(t_instruccion* inst){
	log_info(logger,"La instruccion es %i",inst->nombre);
	list_iterate(inst->parametros, (void*) mostrar_parametro);
}

void mostrar_parametro(char* value){
	log_info(logger,"Parametro %s",value);
}

t_pcb* crear_pcb(t_buffer* buffer){
	t_pcb* pcb = malloc(sizeof(t_pcb));
	int pid;

	int offset = 0;

	memcpy(&(pid),buffer->stream + offset,sizeof(int));
	offset += sizeof(int);

	pcb->pid = pid;
	t_buffer* buffer_i = malloc(sizeof(t_buffer));

	memcpy(&(buffer_i->size), buffer->stream + offset, sizeof(uint32_t));
	offset += sizeof(uint32_t);

	buffer_i->stream = malloc(buffer_i->size);
	memcpy(buffer_i->stream, buffer->stream + offset, buffer_i->size);
	offset += buffer_i->size;

	t_list* lista_instrucciones = deserializar_lista_instrucciones(buffer_i);
	pcb->lista_instrucciones = lista_instrucciones;
	pcb->program_counter = 0;
	pcb->segmentos.id = 0;
	pcb->segmentos.direccion_base = 0;
	pcb->segmentos.tamanio = 0;
	pcb->estimacion = datos.est_inicial;
	//TODO Falta lo de archivos abiertos
	return pcb;
}



///////PLANIFICACION
void agregar_a_cola_new(t_pcb*pcb){
	//sem_wait(&mutex_cola_new);
	queue_push(cola->cola_new_fifo,pcb);
	//sem_post(&mutex_cola_new);
	//sem_post(&sem_new);

}

void agregar_a_cola_ready(t_pcb*pcb){
	//switch(datos.algoritmo_planificacion){
	//case FIFO:
	if(queue_size(cola->cola_ready_fifo)<datos.multiprogramacion){
		queue_push(cola->cola_ready_fifo,pcb);
		queue_pop(cola->cola_new_fifo);} //añadir semaforos
	//	break;
/*	case HRRN:	if(queue_size(cola->cola_ready_fifo)<datos.multiprogramacion){
		agregar_a_cola_hrrn(pcb);}//si no hay espacio tirar log de lleno
		break;
	default: break;
	}*/
}

algoritmo devolver_algoritmo(char*nombre){
	algoritmo alg;
	if(strcmp(nombre,"FIFO")==0) alg=FIFO;
	else if(strcmp(nombre,"HRRN")==0) alg=HRRN;
	else log_info(logger,"algoritmo desconocido");
	return alg;
}

//void agregar_a_cola_hrrn(t_pcb*pcb){

//}

void mostrar_cola(t_queue*cola){
	for(int i=0;i<queue_size(cola);i++){
		int*id=list_get(cola->elements,i);
		log_info(logger,"%d PID: %d",i+1,*id);
	}
	log_info(logger,"-----------------------");
}


t_pcb* quitar_de_new_fifo(){
	t_pcb*pcb=malloc(sizeof(t_pcb));
	pcb=queue_pop(cola->cola_new_fifo);
	return pcb;
}
/*
void quitar_de_ready_fifo(t_pcb*pcb){
	queue_pop(cola->cola_new_fifo,pcb);
}
*/
void finalizar_proceso(t_pcb*pcb){

}



