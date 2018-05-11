CC= g++
CFLAGS = -std=c++11
LIBS= -lEGL -lGL -lIL

EXEC= test

all: $(EXEC)

test: src/test.cpp include/test.h
	$(CC) $(CFLAGS) src/test.cpp -o $@ ${LIBS}

clean:
	rm -rf $(EXEC)
