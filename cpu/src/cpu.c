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
t_pcb* ejecutar_pcb(t_pcb* pcb,int conexion_kernel);
t_instruccion* fetch(t_pcb* pcb);
int decode(t_instruccion* instruccion);
void execute(t_instruccion* instruccion,t_pcb* pcb,int conexion_kernel);
void mostrar(t_instruccion* inst);
void mostrar_parametro(char* value);
registros_pos devolver_registro(char* registro);
void insertar(t_pcb* pcb, registros_pos pos,char* caracteres);
void mostrar_registro(t_pcb* pcb);
int band_ejecutar;

int main(void) {
	logger = iniciar_logger("cpu.log","CPU");;
	config = iniciar_config("cpu.config");
	band_ejecutar = 0;

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
	server_fd = iniciar_servidor(logger,datos.puerto_escucha);
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
					//list_iterate(pcb->lista_instrucciones, (void*) mostrar);
					ejecutar_pcb(pcb,*cliente_fd);


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
					log_destroy(logger);
					config_destroy(config);
					close(*cliente_fd);
					close(server_fd);
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

t_pcb* ejecutar_pcb(t_pcb* pcb,int conexion_kernel){


	t_instruccion* instruccion;

	do{

		instruccion = fetch(pcb);
			if(decode(instruccion) == 1){
				log_info(logger,"Instruccion set");
				usleep(datos.ret_instruccion*1000);

			}
			execute(instruccion,pcb,conexion_kernel);
	}while(band_ejecutar == 0);

	band_ejecutar = 0;


	log_info(logger,"TERMINO LA EJECUCION");
	return pcb;
}

t_instruccion* fetch(t_pcb* pcb){
	t_instruccion* instruccion = list_get(pcb->lista_instrucciones,pcb->program_counter);
	pcb->program_counter+=1;
	log_info(logger,"Program COutnter: %i",pcb->program_counter);
	return instruccion;
}

int decode(t_instruccion* instruccion){
	int acceso = 0;

	if(instruccion->nombre == SET){
		acceso = 1;
	}

	return acceso;
}

void execute(t_instruccion* instruccion,t_pcb* pcb,int conexion_kernel){
	uint32_t result;
	switch(instruccion->nombre){
		case SET:
			log_info(logger,"Paso por Set");
			char* reg_des = list_get(instruccion->parametros,0);
			char* caracteres = list_get(instruccion->parametros,1);
			registros_pos pos = devolver_registro(reg_des);
			insertar(pcb,pos,caracteres);
			log_info(logger,"En %s esta %s",reg_des,caracteres);
			mostrar_registro(pcb);
			break;
		case IO:
			log_info(logger,"Pasa por I/O");
			enviar_pcb_a(pcb,conexion_kernel,EJECUTAR_IO);
			band_ejecutar = 1;
			break;
		case WAIT:
			log_info(logger,"Pasa por Wait");
			enviar_pcb_a(pcb,conexion_kernel,EJECUTAR_WAIT);
			recv(conexion_kernel, &result, sizeof(uint32_t), MSG_WAITALL);
			if(result == 0){
				log_info(logger,"El proceso se bloqueo o no existe");
				band_ejecutar = 1;
			} else {
				log_info(logger,"El programa sigue");
			}
			break;
		case SIGNAL:
			log_info(logger,"Pasa por Signal");
			enviar_pcb_a(pcb,conexion_kernel,EJECUTAR_SIGNAL);
			recv(conexion_kernel, &result, sizeof(uint32_t), MSG_WAITALL);
			if(result == 0){
				log_info(logger,"El proceso se bloqueo o no existe");
				band_ejecutar = 1;
			} else {
				log_info(logger,"El programa sigue");
			}
			break;
		case YIELD:
			log_info(logger,"Paso por YIELD");
			band_ejecutar = 1;
			enviar_pcb_a(pcb,conexion_kernel,DESALOJADO);
			break;
		case EXIT:
			log_info(logger,"Paso por Exit");
			band_ejecutar = 1;
			enviar_pcb_a(pcb,conexion_kernel,FINALIZAR);
			break;
		default:
			log_info(logger,"Otra cosa");
			break;
	}
}

registros_pos devolver_registro(char* registro){
	registros_pos v;
	if(strcmp(registro,"AX")==0){
		v = AX;
	} else if(strcmp(registro,"BX")==0){
		v = BX;
	} else if(strcmp(registro,"CX")==0){
		v = CX;
	} else if(strcmp(registro,"DX")==0){
		v = DX;
	} else if(strcmp(registro,"EAX")==0){
		v = EAX;
	} else if(strcmp(registro,"EBX")==0){
		v = EBX;
	} else if(strcmp(registro,"ECX")==0){
		v = ECX;
	} else if(strcmp(registro,"EDX")==0){
		v = EDX;
	} else if(strcmp(registro,"RAX")==0){
		v = RAX;
	} else if(strcmp(registro,"RBX")==0){
		v = RBX;
	} else if(strcmp(registro,"RCX")==0){
		v = RCX;
	} else if(strcmp(registro,"RDX")==0){
		v = RDX;
	} else {
		log_error(logger,"CUIDADO,CODIGO INVALIDO");
	}
	return v;
}

void insertar(t_pcb* pcb, registros_pos pos,char* caracteres){
	switch(pos){
		case AX: memcpy(pcb->registro.AX,caracteres,5);
			break;
		case BX: memcpy(pcb->registro.BX,caracteres,5);
			break;
		case CX: memcpy(pcb->registro.CX,caracteres,5);
			break;
		case DX: memcpy(pcb->registro.DX,caracteres,5);
			break;
		case EAX: memcpy(pcb->registro.EAX,caracteres,9);
			break;
		case EBX: memcpy(pcb->registro.EBX,caracteres,9);
			break;
		case ECX: memcpy(pcb->registro.ECX,caracteres,9);
			break;
		case EDX: memcpy(pcb->registro.EDX,caracteres,9);
			break;
		case RAX: memcpy(pcb->registro.RAX,caracteres,17);
			break;
		case RBX: memcpy(pcb->registro.RBX,caracteres,17);
			break;
		case RCX: memcpy(pcb->registro.RCX,caracteres,17);
			break;
		case RDX: memcpy(pcb->registro.RDX,caracteres,17);
			break;
	}

}

void mostrar_registro(t_pcb* pcb){


	log_info(logger,"En el registro AX esta los caracteres: %s",pcb->registro.AX);
	log_info(logger,"En el registro BX esta los caracteres: %s",pcb->registro.BX);
	log_info(logger,"En el registro CX esta los caracteres: %s",pcb->registro.CX);
	log_info(logger,"En el registro DX esta los caracteres: %s",pcb->registro.DX);
	log_info(logger,"En el registro EAX esta los caracteres: %s",pcb->registro.EAX);
	log_info(logger,"En el registro EBX esta los caracteres: %s",pcb->registro.EBX);
	log_info(logger,"En el registro ECX esta los caracteres: %s",pcb->registro.ECX);
	log_info(logger,"En el registro EDX esta los caracteres: %s",pcb->registro.EDX);
	log_info(logger,"En el registro RAX esta los caracteres: %s",pcb->registro.RAX);
	log_info(logger,"En el registro RBX esta los caracteres: %s",pcb->registro.RBX);
	log_info(logger,"En el registro RCX esta los caracteres: %s",pcb->registro.RCX);
	log_info(logger,"En el registro RDX esta los caracteres: %s",pcb->registro.RDX);

}
