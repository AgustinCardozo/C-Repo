################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/subdir.mk
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

OPTIONAL_TOOL_DEPS := \
$(wildcard ../makefile.defs) \
$(wildcard ../makefile.init) \
$(wildcard ../makefile.targets) \


BUILD_ARTIFACT_NAME := compartido
BUILD_ARTIFACT_EXTENSION := so
BUILD_ARTIFACT_PREFIX := lib
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: main-build

# Main-build Target
main-build: libcompartido.so

# Tool invocations
libcompartido.so: $(OBJS) $(USER_OBJS) makefile $(OPTIONAL_TOOL_DEPS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc -shared -o "libcompartido.so" $(OBJS) $(USER_OBJS) $(LIBS) -lcommons -lreadline
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) libcompartido.so
	-@echo ' '
	
install: all
	sudo cp -u build/libcompartido.so /usr/lib
	sudo cp --parents -u $(H_SRCS) /usr/include
	
uninstall:
	sudo rm -f /usr/lib/libcompartido.so
	sudo rm -rf /usr/include/compartido

.PHONY: all clean dependents main-build

-include ../makefile.targets
