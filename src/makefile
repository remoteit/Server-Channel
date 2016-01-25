#
# Server Channel Buld File
#

TARGET	= schannel 

all: $(TARGET)

# Which compiler
CC = gcc

STRIP=strip
BASEDIR=

LIBDIR=$(BASEDIR)/lib/
INCDIR=$(BASEDIR)/inc/

# Where are include files kept
INCLUDE = 


# Use this for Linux client software
CDEFINE = -DLINUX 

# Options for development
CFLAGS		= $(INCLUDE) $(CDEFINE) -g -Wall -Os 
LDFLAGS		=
LINKFLAGS	= -o

# add libs for lib crypto TBD
#LIBS		=  -lcrypto -lssl

# Options for release
# CFLAGS = -O -Wall -ansi

#Dependancies for all
DEPENDALL	= config.h mytypes.h debug.h 

#Objects put all your .c files here but with .o extentions
OBJ	= server_channel.o server_channel_packet.o arch.o daemonize.o net.o webio.o yselect.o log.o

$(TARGET): $(OBJ) $(DEPENDALL) 
	$(CC) $(LINKFLAGS) $(TARGET) $(OBJ) $(LIBS)
	$(STRIP) $(TARGET)

clean:
	-rm *.o $(MYLIB)
	 rm -f *.map
	 rm -f $(TARGET)


