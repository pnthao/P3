################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/AQSIOSCoordinator.cpp \
../src/load_manager.cpp 

CC_SRCS += \
../src/file_source.cc \
../src/gen_output.cc \
../src/node_info.cc \
../src/script_file_reader.cc 

OBJS += \
./src/AQSIOSCoordinator.o \
./src/file_source.o \
./src/gen_output.o \
./src/load_manager.o \
./src/node_info.o \
./src/script_file_reader.o 

CC_DEPS += \
./src/file_source.d \
./src/gen_output.d \
./src/node_info.d \
./src/script_file_reader.d 

CPP_DEPS += \
./src/AQSIOSCoordinator.d \
./src/load_manager.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I"/home/thao/workspace/p3/AQSIOSCoordinator/include" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/%.o: ../src/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I"/home/thao/workspace/p3/AQSIOSCoordinator/include" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


