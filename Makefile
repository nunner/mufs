LDLIBS = -lsqlite3
LDLIBS += -lm
LDLIBS += -lz
LDLIBS += $(shell pkg-config fuse3 --libs)
LDLIBS += $(shell pkg-config taglib_c --libs)

LDFLAGS += $(shell pkg-config fuse3 --cflags)
LDFLAGS += $(shell pkg-config taglib_c --cflags)

CFLAGS=-g -Wall -Werror -O3 -fsanitize=undefined -D _GNU_SOURCE #-fsanitize=address 
CC=gcc

SRC=$(wildcard src/*.c)

TARGET=mufs
DB=~/.local/share/mufs/mufs.db

all: $(TARGET) create_db

run: $(TARGET)
	./$(TARGET) root mount

$(TARGET): $(SRC)
	$(CC) -o $(TARGET) $(CFLAGS) $(LDFLAGS) $^ $(LDLIBS)

debug: $(TARGET)
	ulimit -c unlimited
	./$(TARGET) -f root mount

create_db:
	rm ~/.local/share/mufs/mufs.db
	mkdir -p ~/.local/share/mufs
	cat scheme.sql | sqlite3 $(DB)

install:
	sudo cp $(TARGET) /usr/local/bin/$(TARGET)

update: clean all install

clean:
	rm -f $(DB)
	rm -f $(TARGET)
