#ifndef CONSOLA_H_
#define CONSOLA_H_

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>

// #include "utils.h"

t_config* config;
t_log* logger;

// void terminar_programa(int, t_log*, t_config*);
void terminar_programa(t_log*, t_config*);

#endif /* CONSOLA_H_ */
