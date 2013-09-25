################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../src/parser/nodes.cc \
../src/parser/parse.cc \
../src/parser/scan.cc \
../src/parser/scanhelp.cc 

OBJS += \
./src/parser/nodes.o \
./src/parser/parse.o \
./src/parser/scan.o \
./src/parser/scanhelp.o 

CC_DEPS += \
./src/parser/nodes.d \
./src/parser/parse.d \
./src/parser/scan.d \
./src/parser/scanhelp.d 


# Each subdirectory must supply rules for building sources it contributes
src/parser/%.o: ../src/parser/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I"/home/thao/workspace/p3/AQSIOSCoordinator/include" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


