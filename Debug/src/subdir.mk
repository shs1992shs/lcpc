################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/Coords.cpp \
../src/Edge.cpp \
../src/Funnel.cpp \
../src/lcpc.cpp 

OBJS += \
./src/Coords.o \
./src/Edge.o \
./src/Funnel.o \
./src/lcpc.o 

CPP_DEPS += \
./src/Coords.d \
./src/Edge.d \
./src/Funnel.d \
./src/lcpc.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


