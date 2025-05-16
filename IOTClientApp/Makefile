LIBS= -lpthread -lm
SRCS= IOTClient2.c colorimetro.c accelerometro.c 
BIN=client2
CFLAGS+= -g -O0
OBJS=$(subst .cpp,.o,$(SRCS))

all : $(BIN)

$(BIN): $(OBJS)
	@echo [link] $@
	$(CXX) -o $@ $(OBJS) $(LDFLAGS) $(LIBS)

%.o: %.cpp
	@echo [Compile] $<
	$(CXX) -c $(CFLAGS) $< -o $@

clean:
	@rm -f $(OBJS) $(BIN)
