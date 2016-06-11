################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: UMC

dependents:
	-cd /home/utnso/Escritorio/TPOperativos/tp-2016-1c-Explosive-code/tpsolib/Debug && $(MAKE) clean all

# Tool invocations
UMC: $(OBJS) $(USER_OBJS) /home/utnso/Escritorio/TPOperativos/tp-2016-1c-Explosive-code/tpsolib/Debug/libtpsolib.so
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc -L"/home/utnso/Escritorio/TPOperativos/tp-2016-1c-Explosive-code/tpsolib/Debug" -o "UMC" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) UMC
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:
/home/utnso/Escritorio/TPOperativos/tp-2016-1c-Explosive-code/tpsolib/Debug/libtpsolib.so:

-include ../makefile.targets
