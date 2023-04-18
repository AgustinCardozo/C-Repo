/*
 * serializacion.h
 *
 *  Created on: Apr 18, 2023
 *      Author: utnso
 */

#ifndef SRC_SERIALIZACION_H_
#define SRC_SERIALIZACION_H_

#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netdb.h>
#include<commons/log.h>
#include<commons/collections/list.h>
#include<string.h>
#include<assert.h>
#include "compartido.h"

t_paquete* crear_paquete_pcb(t_pcb* pcb, op_code codigo);
void enviar_pcb(t_paquete* paquete,int conexion);
t_buffer* serializar_lista_de_instrucciones(t_list* lista_instrucciones);
t_buffer* serializar_pcb(t_pcb* pcb);
t_list* deserializar_lista_instrucciones(t_buffer* buffer);

#endif /* SRC_SERIALIZACION_H_ */
