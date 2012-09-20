################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/hiredis/async.c \
../src/hiredis/dict.c \
../src/hiredis/example-libev.c \
../src/hiredis/example-libevent.c \
../src/hiredis/example.c \
../src/hiredis/hiredis.c \
../src/hiredis/main.c \
../src/hiredis/net.c \
../src/hiredis/sds.c \
../src/hiredis/test.c 

OBJS += \
./src/hiredis/async.o \
./src/hiredis/dict.o \
./src/hiredis/example-libev.o \
./src/hiredis/example-libevent.o \
./src/hiredis/example.o \
./src/hiredis/hiredis.o \
./src/hiredis/main.o \
./src/hiredis/net.o \
./src/hiredis/sds.o \
./src/hiredis/test.o 

C_DEPS += \
./src/hiredis/async.d \
./src/hiredis/dict.d \
./src/hiredis/example-libev.d \
./src/hiredis/example-libevent.d \
./src/hiredis/example.d \
./src/hiredis/hiredis.d \
./src/hiredis/main.d \
./src/hiredis/net.d \
./src/hiredis/sds.d \
./src/hiredis/test.d 


# Each subdirectory must supply rules for building sources it contributes
src/hiredis/%.o: ../src/hiredis/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


