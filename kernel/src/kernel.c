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
t_pcb* crear_pcb(t_buffer* buffer, int conexion_consola);
void enviar_pcb_a(t_pcb* pcb,int conexion, op_code codigo);
void* atender_cpu(void);
void* de_new_a_ready(void);
void de_ready_a_ejecutar_fifo(void);
void* de_ready_a_ejecutar(void);

/////PLANIFICACION///////
//colas
t_cola cola;

////TRANSICIONES////
void agregar_a_cola_new(t_pcb*pcb);
void agregar_a_cola_ready(t_pcb*pcb);
//void agregar_a_cola_hrrn(t_pcb*pcb);
void finalizar_proceso(t_pcb*pcb);
t_pcb* quitar_de_cola_new();
t_pcb* quitar_de_cola_ready();
algoritmo devolver_algoritmo(char*nombre);

sem_t mutex_cola_new;
sem_t mutex_cola_ready;
sem_t sem_multiprogramacion;
sem_t sem_nuevo;
sem_t sem_habilitar_exec;

pthread_t thread_nuevo_a_ready;
pthread_t thread_atender_cpu;
pthread_t hilo_atender_consolas;
pthread_t thread_ejecutar;
pthread_t thread_recurso;

t_list* lista_recursos;

void* iniciar_recurso(void* data);
int pid;

int main(void) {
	pid=0;

	lista_recursos = list_create();

	cola.cola_new = queue_create();
	cola.cola_ready_fifo=queue_create();
	cola.cola_ready_hrrn=queue_create();

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
	datos.recursos = string_get_string_as_array(config_get_string_value(config,"RECURSOS"));
	datos.instancias = string_get_string_as_array(config_get_string_value(config,"INSTANCIAS_RECURSOS"));

	sem_init(&mutex_cola_new,0,1);
	sem_init(&mutex_cola_ready,0,1);
	sem_init(&sem_multiprogramacion,0,datos.multiprogramacion);
	sem_init(&sem_nuevo,0,0);
	sem_init(&sem_habilitar_exec,0,1);

	int i = 0;
	while(datos.recursos[i] != NULL){
		t_recurso* recurso = malloc(sizeof(t_recurso));

		recurso->nombre = datos.recursos[i];
		recurso->instancias = atoi(datos.instancias[i]);
		recurso->cola_recurso = queue_create();
		sem_init(&recurso->sem_recurso,0,0);

		pthread_create(&thread_recurso,NULL,(void*) iniciar_recurso,recurso);
		list_add(lista_recursos,recurso);

		i++;

	}

	conexion_cpu = crear_conexion(datos.ip_cpu,datos.puerto_cpu);
	conexion_memoria = crear_conexion(datos.ip_memoria,datos.puerto_memoria);
	conexion_filesystem = crear_conexion(datos.ip_filesystem,datos.puerto_filesystem);

	enviar_mensaje("Hola te estoy hablando desde el kernel",conexion_cpu);
	enviar_mensaje("Hola te estoy hablando desde el kernel",conexion_memoria);
	enviar_mensaje("Hola te estoy hablando desde el kernel",conexion_filesystem);

	pthread_create(&thread_atender_cpu,NULL,(void*) atender_cpu,NULL);
	pthread_create(&thread_nuevo_a_ready,NULL,(void*) de_new_a_ready,NULL);
	pthread_create(&thread_ejecutar,NULL,(void*) de_ready_a_ejecutar,NULL);
	server_fd = iniciar_servidor(logger,"127.0.0.1",datos.puerto_escucha);
	log_info(logger, "KERNEL listo para recibir a las consolas");

	cliente_fd = malloc(sizeof(int));//

	while(1){
		cliente_fd = esperar_cliente(logger,server_fd);
		pthread_create(&hilo_atender_consolas,NULL,(void*) atender_consolas,cliente_fd);
		pthread_detach(hilo_atender_consolas);
	}
	pthread_detach(thread_recurso);


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
				case DESALOJADO:
					log_info(logger,"Paso por desalojado");
					sem_post(&sem_habilitar_exec);
					buffer = desempaquetar(paquete,conexion_cpu);
					pcb = deserializar_pcb(buffer);
					agregar_a_cola_ready(pcb);
					break;
				case FINALIZAR:
					log_info(logger,"Paso por finzalizar");
					sem_post(&sem_habilitar_exec);
					buffer = desempaquetar(paquete,conexion_cpu);
					pcb = deserializar_pcb(buffer);
					log_info(logger,"El pcb [%i] ha sido finalizado",pcb->pid);
					enviar_mensaje("Tu proceso ha finalizado",pcb->conexion_consola);

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
				t_pcb* pcb= crear_pcb(buffer,cliente_fd);
				/*if(pcb->pid>0){
					agregar_a_cola_new(pcb);
				}*/
				/*agregar_a_cola_ready(pcb);
				mostrar_cola(cola->cola_ready_fifo);
				mostrar_cola(cola->cola_ready_hrrn);
*/
				log_info(logger,"EL PID QUE ME LLEGO ES %i",pcb->pid);

				agregar_a_cola_new(pcb);
				//mostrar_cola(cola->cola_new_fifo);


				//list_iterate(pcb->lista_instrucciones, (void*) mostrar);

				//enviar_pcb_a(pcb,conexion_cpu,EJECUTAR);


				//log_info(logger,"El alfa es %d",pcb->estimacion);
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

t_pcb* crear_pcb(t_buffer* buffer,int conexion_cliente){
	t_pcb* pcb = malloc(sizeof(t_pcb));

	pid++;
	pcb->pid = pid;

	t_list* lista_instrucciones = deserializar_lista_instrucciones(buffer);
	pcb->lista_instrucciones = lista_instrucciones;
	pcb->program_counter = 0;
	pcb->segmentos.id = 0;
	pcb->segmentos.direccion_base = 0;
	pcb->segmentos.tamanio = 0;
	pcb->estimacion = datos.est_inicial;
	pcb->conexion_consola = conexion_cliente;
	//TODO Falta lo de archivos abiertos
	return pcb;
}



///////PLANIFICACION
void agregar_a_cola_new(t_pcb*pcb){
	sem_wait(&mutex_cola_new);
	queue_push(cola.cola_new,pcb);
	sem_post(&mutex_cola_new);
	log_info(logger,"El proceso [%d] ingreso a la cola nuevo",pcb->pid);
	sem_post(&sem_nuevo);

}

t_pcb* quitar_de_cola_new(){
	//t_pcb* pcb = malloc(sizeof(t_pcb));
	sem_wait(&mutex_cola_new);
	t_pcb* pcb = queue_pop(cola.cola_new);
	sem_post(&mutex_cola_new);
	log_info(logger,"El proceso [%d] fue quitado a la cola nuevo",pcb->pid);
	return pcb;
}


void agregar_a_cola_ready(t_pcb* pcb){
	log_info(logger,"El proceso [%d] fue agregado a la cola ready",pcb->pid);
	sem_wait(&mutex_cola_ready);
	queue_push(cola.cola_ready_fifo,pcb);
	sem_post(&mutex_cola_ready);
	//sem_post(&sem_exec);
	//mostrar_cola(cola.cola_ready);
}

t_pcb* quitar_de_cola_ready(){
	//t_pcb* pcb = malloc(sizeof(t_pcb));
	sem_wait(&mutex_cola_ready);
	t_pcb* pcb = queue_pop(cola.cola_ready_fifo);
	sem_post(&mutex_cola_ready);
	log_info(logger,"El proceso [%d] fue quitado a la cola ready",pcb->pid);
	return pcb;
}

void* de_new_a_ready(void){

	log_info(logger,"nuevo a ready");

	while(1){
		//sem_wait(&sem_avisar);
		sem_wait(&sem_nuevo);
		while(!queue_is_empty(cola.cola_new)){
			sem_wait(&sem_multiprogramacion);
			t_pcb* pcb =quitar_de_cola_new();

			log_info(logger,"“PID: %d - Estado Anterior: NEW - Estado Actual: READY”",pcb->pid);

			agregar_a_cola_ready(pcb);
		}
	}
}


void* de_ready_a_ejecutar(void){
	log_info(logger,"de ready a ejecutar");
	while(1){
		switch(datos.algoritmo_planificacion){
			case FIFO:
				log_info(logger,"Paso por fifo");
				de_ready_a_ejecutar_fifo();
				break;
			case HRRN:

				break;
			default:
				log_warning(logger,"Algo esta ocurriendo en ready a ejectar");
				break;
		}
	}

}

void de_ready_a_ejecutar_fifo(void){
	while(1){
		//sem_wait(&sem_habilitar_exec);
		while(!queue_is_empty(cola.cola_ready_fifo)){
			sem_wait(&sem_habilitar_exec);
			t_pcb* pcb = quitar_de_cola_ready();
			log_info(logger,"“PID: %d - Estado Anterior: READY - Estado Actual: EXEC”",pcb->pid);
			enviar_pcb_a(pcb,conexion_cpu,EJECUTAR);

			//liberar_pcb(pcb);
		}
	}
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

/*
void quitar_de_ready_fifo(t_pcb*pcb){
	queue_pop(cola->cola_new_fifo,pcb);
}
*/
void finalizar_proceso(t_pcb*pcb){

}

//------------------Encarse de los recursos compartidos-------

void* iniciar_recurso(void* data){
	t_recurso* recurso = ((t_recurso*) data);
	log_info(logger,"INICIANDO EL RECURSO [%s]",recurso->nombre);
	while(1){
		sem_wait(&recurso->sem_recurso);
	}
}


