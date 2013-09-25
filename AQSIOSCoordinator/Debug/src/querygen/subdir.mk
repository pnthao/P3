################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../src/querygen/log_naive_plan_gen.cc \
../src/querygen/log_plan_gen.cc \
../src/querygen/logexpr.cc \
../src/querygen/logop.cc \
../src/querygen/sem_interp.cc 

OBJS += \
./src/querygen/log_naive_plan_gen.o \
./src/querygen/log_plan_gen.o \
./src/querygen/logexpr.o \
./src/querygen/logop.o \
./src/querygen/sem_interp.o 

CC_DEPS += \
./src/querygen/log_naive_plan_gen.d \
./src/querygen/log_plan_gen.d \
./src/querygen/logexpr.d \
./src/querygen/logop.d \
./src/querygen/sem_interp.d 


# Each subdirectory must supply rules for building sources it contributes
src/querygen/%.o: ../src/querygen/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I"/home/thao/workspace/p3/AQSIOSCoordinator/include" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


