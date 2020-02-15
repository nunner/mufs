LIBS= -lsqlite3 /usr/lib/libtag_c.so $(shell pkg-config fuse3 --libs)
CFLAGS=-g -Wall -D_FILE_OFFSET_BITS=64 -Wdiscarded-qualifiers -D _GNU_SOURCE

SRC=$(wildcard src/*.c)

TARGET=mufs
DB=~/.local/share/mufs/mufs.db

all: $(TARGET) create_db

$(TARGET): $(SRC)
	gcc -o $(TARGET) $^ $(LIBS) $(CFLAGS)

debug: $(TARGET)
	ulimit -c unlimited
	./$(TARGET) -f root mount

create_db:
	mkdir -p ~/.local/share/mufs
	cat scheme.sql | sqlite3 $(DB)

install:
	sudo cp $(TARGET) /usr/local/bin/$(TARGET)

update: clean all

clean:
	rm $(DB)