VERSION 	=1.00
CC		=aarch64-himix100-linux-gcc
#特殊代码增加宏条件选择是否编译
DEBUG 		=-DUSE_DEBUG
CFLAGS		=-O 
SOURCES	 	=$(wildcard ./*.c)
INCLUDES  	=
#LIB_NAMES 	=-lfun_a -lfun_so
#LIB_PATH 	=-L./lib
LIB_NAMES 	=-lpthread
LIB_PATH 	=
OBJ			=$(patsubst %.c, %.o, $(SOURCES))
TARGET		=hi_hel_app

#links
$(TARGET):$(OBJ)
	@mkdir -p output
	$(CC) $(OBJ) $(LIB_PATH) $(LIB_NAMES) -o output/$(TARGET)$(VERSION)
	@rm -rf $(OBJ)
	
#compile
%.o: %.c
	$(CC) $(INCLUDES) $(DEBUG) -c $(CFLAGS) $< -o $@

.PHONY:clean
clean:
	@echo "Remove linked and compiled files......"
	rm -rf $(OBJ) $(TARGET) output
