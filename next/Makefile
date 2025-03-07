
CC = gcc              # use cc if gcc not available
CFLAGS = -c -g        # -Wall
STRIP = strip         # delete $(STRIP) line below if unavailable
PROG =  gez2
LIBS = -leth -ll      # it is critical that -leth precedes -ll
#MYLIBDIR = ./lib-src
MYINCLUDE = ./include
SOURCE  = main.c rtfUtils.c docOut.c tables.c
OBJECTS = $(SOURCE:.c=.o)
#LIBRARY = libeth.a
#DEPFLAGS = -DSUPPORT_DOS -DSUPPORT_TEX  -DSUPPORT_IES -DSUPPORT_ISO
DEPFLAGS = -DSUPPORT_TEX  -DSUPPORT_IES -DSUPPORT_ISO



all: ${PROG}
 
${PROG}:  $(OBJECTS)
	    $(CC) -g $(DEPFLAGS) $(OBJECTS) -I$(MYINCLUDE) $(LIBS) -o ${PROG}
#		$(STRIP) $(PROG)


$(LIBRARY):
	(cd $(MYLIBDIR); make -f Makefile)


.c.o:
	$(CC) $(CFLAGS) $(DEPFLAGS) -I$(MYINCLUDE) $*.c

clean:
	    rm -f $(OBJECTS)

cleanall:
	    rm -f $(OBJECTS) $(PROG) $(PROGLINKS)


