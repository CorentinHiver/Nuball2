ROOT_CFLAGS     := $(shell root-config --cflags)
ROOT_LIBS       := $(shell root-config --libs)

exec: $(HEADERS) forDebugOnly.cpp
	g++ -o exec forDebugOnly.cpp $(ROOT_CFLAGS) $(ROOT_LIBS) -g -Og -DDEBUG -Wall -Wextra