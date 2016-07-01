################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/archivoConfig.c \
../src/bitMap.c \
../src/particionSwap.c \
../src/procesosSwap.c \
../src/swap.c \
../src/swapUmc.c 

OBJS += \
./src/archivoConfig.o \
./src/bitMap.o \
./src/particionSwap.o \
./src/procesosSwap.o \
./src/swap.o \
./src/swapUmc.o 

C_DEPS += \
./src/archivoConfig.d \
./src/bitMap.d \
./src/particionSwap.d \
./src/procesosSwap.d \
./src/swap.d \
./src/swapUmc.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"../../tpsolib" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


