RM=rm -rf
CC=gcc

C_SRCS=$(shell find . -iname "*.c" | tr '\n' ' ')
H_SRCS=$(shell find . -iname "*.h" | tr '\n' ' ')
OBJS=$(C_SRCS:./%.c=build/%.o)

ifneq ($(shell id -un),root)
SUDO=sudo
endif

# Clean and compile .so
all: build/libcompartido.so

build:
	mkdir -p $@

build/libcompartido.so: build $(OBJS)
	$(CC) -shared -o "$@" $(OBJS)

build/%.o: %.c
	$(CC) -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"

# Add debug parameters and compile
debug: CC += -DDEBUG -g
debug: all

# Print all sources
sources:
	@echo $(C_SRCS) $(H_SRCS)

# Clean build files
clean:
	$(RM) build

install: all
	$(SUDO) cp -u build/libcompartido.so /usr/lib
	$(SUDO) cp --parents -u $(H_SRCS) /usr/include

uninstall:
	$(SUDO) rm -f /usr/lib/libcompartido.so
	$(SUDO) rm -rf /usr/include/commons

.PHONY: all debug sources clean install uninstall
