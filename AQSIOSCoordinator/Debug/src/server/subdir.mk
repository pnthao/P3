################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../src/server/server_impl.cc 

OBJS += \
./src/server/server_impl.o 

CC_DEPS += \
./src/server/server_impl.d 


# Each subdirectory must supply rules for building sources it contributes
src/server/%.o: ../src/server/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I"/home/thao/workspace/p3/AQSIOSCoordinator/include" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


