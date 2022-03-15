#TOOLCHAIN=~/toolchain/gcc-arm-none-eabi-4_9-2014q4/bin
#PREFIX=$(TOOLCHAIN)/arm-none-eabi-
PREFIX=arm-none-eabi-

ARCHFLAGS=-mthumb -mcpu=cortex-m0plus
COMMONFLAGS=-g3 -O0 -Wall -Werror $(ARCHFLAGS)




SRC_DIR:=/board /board/src /CMSIS /drivers /utilities
I_DIRS := /utilities /drivers /board/src /board /CMSIS
OBJ_DIR := SRC_DIR

CFLAGS=-DCPU_MKL46Z256VLL4 -I./drivers -I./CMSIS -I./board -I./board/src -I./utilities $(COMMONFLAGS)


LDFLAGS=$(COMMONFLAGS) --specs=nano.specs -Wl,--gc-sections,-Map,$(TARGET).map,-Tlink.ld
LDLIBS=

CC=$(PREFIX)gcc
LD=$(PREFIX)gcc
OBJCOPY=$(PREFIX)objcopy
SIZE=$(PREFIX)size
RM=rm -f

TARGET1=led_blinky
TARGET2=hello_world

SRC_L:=$(wildcard CMSIS/*.c board/*.c board/src/*.c drivers/*.c utilities/*.c startup_mkl46z4.c led_blinky.c )
SRC_L:= $(filter-out board/pin_mux_h.c, $(SRC_L))
SRC_L:= $(filter-out hello_world.c, $(SRC_L))
OBJ_L=$(patsubst %.c, %.o, $(SRC_L))

SRC_H:=$(wildcard CMSIS/*.c board/*.c board/src/*.c drivers/*.c utilities/*.c hello_world.c  )
SRC_H:= $(filter-out board/pin_mux_l.c, $(SRC_H))
SRC_H:= $(filter-out led_blinky.c, $(SRC_H))
OBJ_H=$(patsubst %.c, %.o, $(SRC_H))

all: build1 build2 size

build1: elf1  bin
build2: elf2  bin

elf1: $(TARGET1).elf
elf2: $(TARGET2).elf

srec: $(TARGET1).srec

bin: $(TARGET1).bin



clean:
	$(RM) $(TARGET1).srec $(TARGET1).elf $(TARGET1).bin $(TARGET1).map $(OBJ_L) $(TARGET2).srec $(TARGET2).elf $(TARGET2).bin $(TARGET2).map $(OBJ_H)

$(TARGET1).elf: $(OBJ_L)
	$(LD) $(LDFLAGS) $(OBJ_L) $(LDLIBS) -o $@
	
$(TARGET2).elf: $(OBJ_H)
	$(LD) $(LDFLAGS) $(OBJ_H) $(LDLIBS) -o $@


%.bin: %.elf
	    $(OBJCOPY) -O binary $< $@
	    

size:
	$(SIZE) $(TARGET1).elf

flash1: build1
	openocd -f openocd.cfg -c "program $(TARGET1).elf verify reset exit"
	
flash2: build2
	openocd -f openocd.cfg -c "program $(TARGET2).elf verify reset exit"
