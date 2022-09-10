#include "consola.h"

int main(int argc, char** argv){
    // if (argc < 2) {
    //     return EXIT_FAILURE;
    // }
    // config = config_create(argv[1]); //TODO
    config = config_create("./cfg/consola.config");
    char* ip_kernel = config_get_string_value(config, "IP_KERNEL");
    char* puerto_kernel = config_get_string_value(config, "PUERTO_KERNEL");

    logger = log_create("./cfg/consola.log", "consola", true, LOG_LEVEL_INFO); 
    log_info(logger, "IP KERNEL: %s", ip_kernel);
    log_info(logger, "PUERTO KERNEL: %s", puerto_kernel);

    // int conexion = crear_conexion(ip_kernel, puerto_kernel);

    // terminar_programa(conexion, logger, config);
    terminar_programa(logger, config);
}

// void terminar_programa(int conexion, t_log* logger, t_config* config)
void terminar_programa(t_log* logger, t_config* config)
{
	if(logger != NULL)
		log_destroy(logger);
	if(config != NULL)
		config_destroy(config);
	// liberar_conexion(conexion);
}