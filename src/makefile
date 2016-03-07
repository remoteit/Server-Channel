#
# Server Channel Buld File
#

EXEC	= schannel 


# Which compiler
CC = gcc

STRIP=strip
BASEDIR=

LIBDIR=$(BASEDIR)/lib/
INCDIR=$(BASEDIR)/inc/

# Where are include files kept
INCLUDE = 

ARCH=

# Use this for Linux client software
CDEFINE = -DLINUX 

# Options for development
CFLAGS		= $(INCLUDE) $(CDEFINE) $(ARCH) -g -Wall -Os 
LDFLAGS		=
LINKFLAGS	= $(ARCH) -o

ifeq ($(SHARED),yes)
#LDFLAGS=-g
LDFLAGS=-g
else
LDFLAGS+= -v -Wl,-warn-common --static
endif


# add libs for lib crypto TBD
#LIBS		=  -lcrypto -lssl

# Options for release
# CFLAGS = -O -Wall -ansi

#Dependancies for all
DEPENDALL	= config.h mytypes.h debug.h 

#Objects put all your .c files here but with .o extentions
OBJ	= server_channel.o server_channel_packet.o arch.o daemonize.o net.o webio.o log.o file_config.o yhash.o


all: $(EXEC)
	cp $< ../bin/

$(EXEC): $(OBJ) $(DEPENDALL) 
	$(CC) $(LDFLAGS) $(LINKFLAGS) $(EXEC) $(OBJ) $(LIBS)
	cp $@ $@_g
	$(STRIP) $@

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	-rm *.o $(MYLIB)
	 rm -f *.map
	 rm -f $(TARGET)

