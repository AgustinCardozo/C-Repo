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
void mostrar_cola(t_queue*cola);
t_pcb* crear_pcb(int pid, t_list* lista_instrucciones);
t_buffer* serializar_pcb(t_pcb* pcb);
t_paquete* crear_paquete_pcb(t_pcb* pcb, op_code codigo);
void enviar_pcb_a_ajecutar(t_pcb* pcb);
void enviar_pcb(t_paquete* paquete,int conexion);
t_buffer* serializar_lista_de_instrucciones(t_list* lista_instrucciones);

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
				t_pcb* pcb= crear_pcb(pid,lista_instrucciones);
				if(pcb->pid>0){
					agregar_a_cola_new(pcb);
				}
				/*agregar_a_cola_ready(pcb);
				mostrar_cola(cola->cola_ready_fifo);
				mostrar_cola(cola->cola_ready_hrrn);
*/
				log_info(logger,"EL PID QUE ME LLEGO ES %i",pcb->pid);
				mostrar_cola(cola->cola_new_fifo);


				//list_iterate(pcb->lista_instrucciones, (void*) mostrar);
				paquete = crear_paquete_pcb(pcb,EJECUTAR);

				enviar_pcb(paquete,conexion_cpu);

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

t_pcb* crear_pcb(int pid, t_list* lista_instrucciones){
	t_pcb* pcb = malloc(sizeof(t_pcb));
	pcb->pid = pid;
	pcb->lista_instrucciones = lista_instrucciones;
	pcb->program_counter = 0;
	pcb->segmentos.id = 0;
	pcb->segmentos.direccion_base = 0;
	pcb->segmentos.tamanio = 0;
	pcb->estimacion = datos.est_inicial;
	//TODO Falta lo de archivos abiertos
	return pcb;
}

t_buffer* serializar_pcb(t_pcb* pcb){
	t_buffer* buffer = malloc(sizeof(t_buffer));
	t_buffer* instrucciones = serializar_lista_de_instrucciones(pcb->lista_instrucciones);
	int offset = 0;
	buffer->size = sizeof(int)*8 + instrucciones->size;

	buffer->stream = malloc(buffer->size);

	memcpy(buffer->stream + offset, &pcb->pid, sizeof(int));
	offset += sizeof(int);

	memcpy(buffer->stream + offset, &pcb->program_counter, sizeof(int));
	offset += sizeof(int);

	memcpy(buffer->stream + offset, &pcb->segmentos.id, sizeof(int));
	offset += sizeof(int);

	memcpy(buffer->stream + offset, &pcb->segmentos.direccion_base, sizeof(int));
	offset += sizeof(int);

	memcpy(buffer->stream + offset, &pcb->segmentos.tamanio, sizeof(int));
	offset += sizeof(int);

	memcpy(buffer->stream + offset, &pcb->estimacion, sizeof(int));
	offset += sizeof(int);

	memcpy(buffer->stream + offset, &(instrucciones->size), sizeof(int));
	offset += sizeof(int);

	if (instrucciones->size > 0) {
		memcpy(buffer->stream + offset, instrucciones->stream, instrucciones->size);
		offset += instrucciones->size;
	}

	return buffer;

}

t_paquete* crear_paquete_pcb(t_pcb* pcb, op_code codigo){
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = codigo;
	paquete->buffer = serializar_pcb(pcb);

	return paquete;
}

void enviar_pcb_a_ajecutar(t_pcb* pcb){
	log_info(logger,"HOLAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
	t_paquete* paquete = crear_paquete_pcb(pcb,EJECUTAR);

	enviar_pcb(paquete,conexion_cpu);
}

void enviar_pcb(t_paquete* paquete,int conexion){
	void* a_enviar = malloc(paquete->buffer->size + sizeof(op_code) + sizeof(uint32_t));
	int offset = 0;

	memcpy(a_enviar + offset, &(paquete->codigo_operacion), sizeof(op_code));

	offset += sizeof(op_code);
	memcpy(a_enviar + offset, &(paquete->buffer->size), sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(a_enviar + offset, paquete->buffer->stream, paquete->buffer->size);

	send(conexion, a_enviar, paquete->buffer->size + sizeof(op_code) + sizeof(uint32_t), 0);

	free(a_enviar);
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}


t_buffer* serializar_lista_de_instrucciones(t_list* lista_instrucciones){
	t_buffer* buffer = malloc(sizeof(t_buffer));
	int i_count;
	int	p_count;
	int stream_size;
	int offset = 0;
	t_list_iterator* i_iterator;
	t_list_iterator* p_iterator;
	t_instruccion*	instruccion;
	char* parametro;
	uint32_t size_parametro;

	i_iterator = list_iterator_create(lista_instrucciones);
	stream_size = 0;
	i_count = 0;

	while (list_iterator_has_next(i_iterator)) {
			instruccion = (t_instruccion*)list_iterator_next(i_iterator);

			// codigo, cantidad de parametros
			stream_size += (sizeof(uint32_t) * 2);
			p_count = list_size(instruccion->parametros);

			if (p_count > 0) {
				p_iterator = list_iterator_create(instruccion->parametros);

				while (list_iterator_has_next(p_iterator)) {
					// longitud del parametro, valor del parametro
					stream_size += sizeof(uint32_t) + strlen((char*)list_iterator_next(p_iterator)) + 1;
				}

				list_iterator_destroy(p_iterator);
			}

			i_count++;
		}

	list_iterator_destroy(i_iterator);

	buffer->size = stream_size + sizeof(uint32_t) + sizeof(int);
	buffer->stream = malloc(buffer->size);

	memcpy(buffer->stream + offset, &(i_count), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	i_iterator = list_iterator_create(lista_instrucciones);

	while (list_iterator_has_next(i_iterator)) {
			instruccion = (t_instruccion*)list_iterator_next(i_iterator);

			// codigo de instruccion
			memcpy(buffer->stream + offset, &(instruccion->nombre), sizeof(uint32_t));
			offset += sizeof(uint32_t);

			// cantidad de parametros
			p_count = list_size(instruccion->parametros);
			memcpy(buffer->stream + offset, &(p_count), sizeof(uint32_t));
			offset += sizeof(uint32_t);

			if (p_count > 0) {
				p_iterator = list_iterator_create(instruccion->parametros);

				while (list_iterator_has_next(p_iterator)) {
					parametro = (char*)list_iterator_next(p_iterator);
					size_parametro = strlen(parametro) + 1;

					// longitud del parametro
					memcpy(buffer->stream + offset, &(size_parametro), sizeof(uint32_t));
					offset += sizeof(uint32_t);

					// valor del parametro
					memcpy(buffer->stream + offset, parametro, size_parametro);
					offset += size_parametro;
				}

				list_iterator_destroy(p_iterator);
			}
		}

	list_iterator_destroy(i_iterator);
	if (offset != buffer->size){
			printf("serializar_lista_instrucciones: El tamaño del buffer[%d] no coincide con el offset final[%d]\n", buffer->size, offset);
	}
	return buffer;

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
	case FEEDBACK:
		break;
	default: break;
	}*/
}

algoritmo devolver_algoritmo(char*nombre){
	algoritmo alg;
	if(strcmp(nombre,"FIFO")==0) alg=FIFO;
	else if(strcmp(nombre,"HRRN")==0) alg=HRRN;
	else if(strcmp(nombre,"FEEDBACK")==0) alg=FEEDBACK;
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



