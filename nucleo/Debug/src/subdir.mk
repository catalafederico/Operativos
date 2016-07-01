################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/configuracionesNucleo.c \
../src/funcionesparsernuevas.c \
../src/nucleo.c \
../src/procesarPrograma.c \
../src/procesosCPU.c \
../src/procesosConsola.c \
../src/procesosUMC.c 

OBJS += \
./src/configuracionesNucleo.o \
./src/funcionesparsernuevas.o \
./src/nucleo.o \
./src/procesarPrograma.o \
./src/procesosCPU.o \
./src/procesosConsola.o \
./src/procesosUMC.o 

C_DEPS += \
./src/configuracionesNucleo.d \
./src/funcionesparsernuevas.d \
./src/nucleo.d \
./src/procesarPrograma.d \
./src/procesosCPU.d \
./src/procesosConsola.d \
./src/procesosUMC.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"../../tpsolib/Debug" -I"../../tpsolib" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


