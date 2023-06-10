################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/compartido.c \
../src/serializacion.c \
../src/sockets.c \
../src/utils.c 

C_DEPS += \
./src/compartido.d \
./src/serializacion.d \
./src/sockets.d \
./src/utils.d 

OBJS += \
./src/compartido.o \
./src/serializacion.o \
./src/sockets.o \
./src/utils.o 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c src/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src

clean-src:
	-$(RM) ./src/compartido.d ./src/compartido.o ./src/serializacion.d ./src/serializacion.o ./src/sockets.d ./src/sockets.o ./src/utils.d ./src/utils.o

.PHONY: clean-src

