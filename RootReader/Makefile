ROOT_CFLAGS     := $(shell root-config --cflags)
ROOT_LIBS       := $(shell root-config --libs)

LIBS := ../lib ./Classes ./Modules

HEADER_DIR := $(shell find $(LIBS) -type d)
HEADER_FILES := $(shell find $(HEADER_DIR) -type f -name '*.hpp' )
HEADER_INCLUDES := $(addprefix -I,$(shell find $(HEADER_DIR) -type d ))

.PHONY: debug opt

%.o: %.cpp ${HEADER_FILES}
	${CC} -c $< ${CFLAGS} ${LIBS} ${HEADER_INCLUDES}

CC    	=	g++
CFLAGS	=	${ROOT_CFLAGS} -pthread -std=c++17 -Wall
LIBS  	=	${ROOT_LIBS}

SRCEXE = ManipConvertor.cpp
EXE := $(SRCEXE:.cpp=)

default: $(EXE)

$(EXE): $(SRCEXE:.cpp=.o) ${HEADER_FILES}
	${CC} $< -o $@ ${CFLAGS} ${LIBS}

opt: CFLAGS += -O2
opt: default

debug: CFLAGS += -g -O0 -DDEBUG -Wextra
debug: default

clean:
	rm -f *.o
	rm -f *__*.*
	rm -f $(EXE)
