################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../src/metadata/gen_phy_plan.cc \
../src/metadata/phy_op.cc \
../src/metadata/plan_mgr.cc \
../src/metadata/plan_mgr_impl.cc \
../src/metadata/plan_trans.cc \
../src/metadata/query_mgr.cc \
../src/metadata/table_mgr.cc 

OBJS += \
./src/metadata/gen_phy_plan.o \
./src/metadata/phy_op.o \
./src/metadata/plan_mgr.o \
./src/metadata/plan_mgr_impl.o \
./src/metadata/plan_trans.o \
./src/metadata/query_mgr.o \
./src/metadata/table_mgr.o 

CC_DEPS += \
./src/metadata/gen_phy_plan.d \
./src/metadata/phy_op.d \
./src/metadata/plan_mgr.d \
./src/metadata/plan_mgr_impl.d \
./src/metadata/plan_trans.d \
./src/metadata/query_mgr.d \
./src/metadata/table_mgr.d 


# Each subdirectory must supply rules for building sources it contributes
src/metadata/%.o: ../src/metadata/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I"/home/thao/workspace/p3/AQSIOSCoordinator/include" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


