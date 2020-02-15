LDLIBS = -lsqlite3
LDLIBS += -lm
LDLIBS += -lz
LDLIBS += $(shell pkg-config fuse3 --libs)
LDLIBS += $(shell pkg-config taglib_c --libs)

LDFLAGS += $(shell pkg-config fuse3 --cflags)
LDFLAGS += $(shell pkg-config taglib_c --cflags)

CFLAGS=-lstdc++ -g -Wall -D_FILE_OFFSET_BITS=64 -Wdiscarded-qualifiers -D _GNU_SOURCE
CC=gcc

SRC=$(wildcard src/*.c)

TARGET=mufs
DB=~/.local/share/mufs/mufs.db

all: $(TARGET) create_db

$(TARGET): $(SRC)
	$(CC) -o $(TARGET) $(CFLAGS) $(LDFLAGS) $^ $(LDLIBS)

debug: $(TARGET)
	ulimit -c unlimited
	./$(TARGET) -f root mount

create_db:
	mkdir -p ~/.local/share/mufs
	cat scheme.sql | sqlite3 $(DB)

install:
	sudo cp $(TARGET) /usr/local/bin/$(TARGET)

update: clean all install

clean:
	rm -f $(DB)
	rm -f $(TARGET)