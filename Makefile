APPS:=xcowsay
OBJS:=xcowsay.o floating_shape.o display_cow.o settings.o
HEADERS:=floating_shape.h display_cow.h settings.h

CFLAGS:=-Wall -g `pkg-config --cflags gtk+-2.0`
LDFLAGS:=`pkg-config --libs gtk+-2.0`
LIBS:=

all: $(APPS)

xcowsay: $(OBJS)
	cc $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

%.o: %.c $(HEADERS)
	cc $(CFLAGS) -c $<

clean:
	rm -f $(BIN) $(OBJS)
