LIBS= -lpthread -lm
SRCS= proyecto.c colorimetro.c accelerometro.c 
BIN=proyecto
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
