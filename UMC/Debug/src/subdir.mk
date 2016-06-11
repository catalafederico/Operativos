################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/archivoConf.c \
../src/umc.c \
../src/umcCliente.c \
../src/umcClock.c \
../src/umcConsola.c \
../src/umcCpu.c \
../src/umcMemoria.c \
../src/umcNucleo.c \
../src/umcServer.c \
../src/umcTlb.c 

OBJS += \
./src/archivoConf.o \
./src/umc.o \
./src/umcCliente.o \
./src/umcClock.o \
./src/umcConsola.o \
./src/umcCpu.o \
./src/umcMemoria.o \
./src/umcNucleo.o \
./src/umcServer.o \
./src/umcTlb.o 

C_DEPS += \
./src/archivoConf.d \
./src/umc.d \
./src/umcCliente.d \
./src/umcClock.d \
./src/umcConsola.d \
./src/umcCpu.d \
./src/umcMemoria.d \
./src/umcNucleo.d \
./src/umcServer.d \
./src/umcTlb.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/Escritorio/TPOperativos/tp-2016-1c-Explosive-code/tpsolib" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


