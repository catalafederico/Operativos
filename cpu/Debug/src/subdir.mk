################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/archivoConf.c \
../src/cpu.c \
../src/funcionesparsernuevas.c 

OBJS += \
./src/archivoConf.o \
./src/cpu.o \
./src/funcionesparsernuevas.o 

C_DEPS += \
./src/archivoConf.d \
./src/cpu.d \
./src/funcionesparsernuevas.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"../../tpsolib" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


