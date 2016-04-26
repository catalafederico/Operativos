################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/basicFunciones.c \
../src/nucleo.c \
../src/socketServer.c 

OBJS += \
./src/basicFunciones.o \
./src/nucleo.o \
./src/socketServer.o 

C_DEPS += \
./src/basicFunciones.d \
./src/nucleo.d \
./src/socketServer.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


