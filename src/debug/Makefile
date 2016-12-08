.SUFFIXES : .x .o .c .s

OBJS=*.o
ODIR=obj
$(ROOT_DIR)/$(BIN_DIR)/$(BIN):$(ODIR)/$(OBJS)
	$(CC) -o $@ $^  -lasound -lpthread -lrt -lfreetype -liconv -lm -ldl  -rdynamic

	cp -a $(ROOT_DIR)/$(BIN_DIR)/$(BIN) /home/tftpboot/

	/usr/local/arm_linux_4.2/arm-none-linux-gnueabi/bin/objdump -d $(ROOT_DIR)/$(BIN_DIR)/$(BIN) > $(ROOT_DIR)/$(BIN_DIR)/a.dump

