ROOT_CFLAGS     := $(shell root-config --cflags)
ROOT_LIBS       := $(shell root-config --libs)
GZLIB_CFLAGS    := -lz

recueil: $(HEADERS) recueil.cpp
	g++ -o recueil recueil.cpp $(ROOT_CFLAGS) $(ROOT_LIBS) $(GZLIB_CFLAGS) -O2 -Wall -Wextra

clean:
	rm -f recueil