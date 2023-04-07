/*
 * compartido.h
 *
 *  Created on: Apr 6, 2023
 *      Author: utnso
 */

#ifndef SRC_COMPARTIDO_H_
#define SRC_COMPARTIDO_H_

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<commons/collections/list.h>
#include<unistd.h>
#include<netdb.h>

typedef enum
{
	MENSAJE,
	PAQUETE,
}op_code;

typedef struct{
	int cod;
	t_list* lista;
}bloque;

typedef struct
{
	int size;
	void* stream;
}t_buffer;

typedef struct
{
	op_code codigo_operacion;
	t_buffer* buffer;
}t_paquete;

void saludo();

#endif /* SRC_COMPARTIDO_H_ */
