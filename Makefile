APPS:=xcowsay
OBJS:=xcowsay.o
HEADERS:=

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
