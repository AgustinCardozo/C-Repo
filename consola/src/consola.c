/*
 ============================================================================
 Name        : consola.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================

 */

#include <stdio.h>
#include <stdlib.h>
#include "consola.h"

t_log* logger;
t_config* config;

t_list* parsear(char* ruta);
t_instruccion* parsear_linea(char* linea);
void mostrar(t_instruccion* inst);
void mostrar_parametro(char* value);
t_paquete* crear_paquete_instrucciones(t_list* lista_instrucciones);
t_buffer* serializar_lista_de_instrucciones(t_list* lista_instrucciones);
void enviar_paquete_de_instrucciones(t_paquete* paquete,int conexion);

int main(void) {

	char* ip;
	char* puerto;
	//char* ruta_arch = "prueba.text";

	logger = iniciar_logger("consola.log","Consola");
	config = iniciar_config("consola.config");
	int conexion;
	ip = config_get_string_value(config,"IP_KERNEL");
	puerto = config_get_string_value(config,"PUERTO_KERNEL");

	log_info(logger,"Esto es la consola y los IP del kernel son %s y el Puerto es %s \n",ip,puerto);

	conexion = crear_conexion(ip,puerto);

	t_list* lista_instrucciones = list_create();

	lista_instrucciones = parsear("prueba.txt");

	list_iterate(lista_instrucciones, (void*) mostrar);

	enviar_mensaje("Hola estoy probando cosas para despues",conexion);
	//paquete(conexion);
	t_paquete* paquete_instrucciones = crear_paquete_instrucciones(lista_instrucciones);
	enviar_paquete_de_instrucciones(paquete_instrucciones,conexion);

	log_destroy(logger);
	config_destroy(config);
	close(conexion); 

	return EXIT_SUCCESS;
}

t_list* parsear(char* ruta){
	char buffer[500]; // Maximo de caracteres?

	t_list* lista = list_create();

	FILE* archivo = fopen(ruta,"r");

	while(fgets(buffer,500,archivo)){
	//while(getline(&buffer,&len,archivo)){


			strtok(buffer, "\n");
			list_add(lista,parsear_linea(buffer));

			//log_info(logger,"%s",buffer);
	}

	fclose(archivo);
	return lista;
}


t_instruccion* parsear_linea(char* linea){
	t_instruccion* inst = malloc(sizeof(t_instruccion));

	inst->parametros = list_create();

	char** linea_parse = string_split(linea," ");
	int i = 1;
	if(strcmp(linea_parse[0],"SET") == 0){
		inst->nombre = SET;

		//list_add(inst->parametros,strdup(linea_parse[1]));
		//list_add(inst->parametros,strdup(linea_parse[2]));
		//list_add(lista,inst);

	} else if(strcmp(linea_parse[0],"MOV_IN")== 0){
		inst->nombre = MOV_IN;

		//list_add(lista,inst);
	} else if(strcmp(linea_parse[0],"MOV_OUT")== 0){
		inst->nombre = MOV_OUT;

		//list_add(lista,inst);
	} else if(strcmp(linea_parse[0],"I/O")== 0){
		inst->nombre = IO;

		//list_add(lista,inst);
	}else if(strcmp(linea_parse[0],"F_OPEN")== 0){
		inst->nombre = F_OPEN;

		//list_add(lista,inst);
	}else if(strcmp(linea_parse[0],"F_CLOSE")== 0){
		inst->nombre = F_CLOSE;

		//list_add(lista,inst);
	}else if(strcmp(linea_parse[0],"F_SEEK")== 0){
		inst->nombre = F_SEEK;

		//list_add(lista,inst);
	}else if(strcmp(linea_parse[0],"F_READ")== 0){
		inst->nombre = F_READ;

		//list_add(lista,inst);
	}else if(strcmp(linea_parse[0],"F_WRITE")== 0){
		inst->nombre = F_WRITE;

		//list_add(lista,inst);
	}else if(strcmp(linea_parse[0],"F_TRUNCATE")== 0){
		inst->nombre = F_TRUNCATE;

		//list_add(lista,inst);
	}else if(strcmp(linea_parse[0],"WAIT")== 0){
		inst->nombre = WAIT;

		//list_add(lista,inst);
	}else if(strcmp(linea_parse[0],"SIGNAL")== 0){
		inst->nombre = SIGNAL;

		//list_add(lista,inst);
	}else if(strcmp(linea_parse[0],"CREATE_SEGMENT")== 0){
		inst->nombre = CREATE_SEGMENT;

		//list_add(lista,inst);
	}else if(strcmp(linea_parse[0],"DELETE_SEGMENT")== 0){
		inst->nombre = DELETE_SEGMENT;

		//list_add(lista,inst);
	}else if(strcmp(linea_parse[0],"YIELD")== 0){
		inst->nombre = YIELD;

		//list_add(lista,inst);
	} else if(strcmp(linea_parse[0],"EXIT")== 0){
		inst->nombre = EXIT;
	}

	while(linea_parse[i] != NULL){
		//log_info(logger,linea_parse[i]);
		list_add(inst->parametros,strdup(linea_parse[i]));
		i++;
		//sleep(5);
	}

	return inst;
	free(inst);
}

void mostrar(t_instruccion* inst){
	log_info(logger,"La instruccion es %i",inst->nombre);
	list_iterate(inst->parametros, (void*) mostrar_parametro);
}

void mostrar_parametro(char* value){
	log_info(logger,"Parametro %s",value);
}

t_paquete* crear_paquete_instrucciones(t_list* lista_instrucciones){
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = INSTRUCCIONES;
	paquete->buffer = serializar_lista_de_instrucciones(lista_instrucciones);

	int pid = getpid();
	log_info(logger,"EL PID ES %i",pid);

	int offset = paquete->buffer->size;

	memcpy(paquete->buffer->stream + offset, &pid, sizeof(int));
	offset += sizeof(int);

	return paquete;
}

void enviar_paquete_de_instrucciones(t_paquete* paquete,int conexion){
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
