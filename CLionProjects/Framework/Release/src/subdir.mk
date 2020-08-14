################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/EF_local_search_solver.cpp \
../src/EF_manager.cpp \
../src/EF_second_stage_solver.cpp \
../src/Example_SSTP.cpp \
../src/SP_solution.cpp \
../src/Instance.cpp \
../src/Solution.cpp \
../src/Greedy.cpp

OBJS += \
./src/EF_local_search_solver.o \
./src/EF_manager.o \
./src/EF_second_stage_solver.o \
./src/Example_SSTP.o \
./src/SP_solution.o \
./src/Instance.o \
./src/Solution.o \
./src/Greedy.o

CPP_DEPS += \
./src/EF_local_search_solver.d \
./src/EF_manager.d \
./src/EF_second_stage_solver.d \
./src/Example_SSTP.d \
./src/SP_solution.d \
./src/Instance.d \
./src/Solution.d \
./src/Greedy.d


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


