################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../sockets/basicFunciones.c \
../sockets/socketCliente.c \
../sockets/socketServer.c 

OBJS += \
./sockets/basicFunciones.o \
./sockets/socketCliente.o \
./sockets/socketServer.o 

C_DEPS += \
./sockets/basicFunciones.d \
./sockets/socketCliente.d \
./sockets/socketServer.d 


# Each subdirectory must supply rules for building sources it contributes
sockets/%.o: ../sockets/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


