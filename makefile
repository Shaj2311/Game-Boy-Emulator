ARGS = -std=c99 -Wall -Wextra -O2 -Iinclude -g
TARGET = gb
VPATH = src

SRCS := $(wildcard src/*.c src/**/*.c)
OBJS := $(SRCS:.c=.o)

all: $(TARGET)

# gb: main.o bitshift.o misc.o
# 	gcc $(ARGS) main.o bitshift.o misc.o -o gb
$(TARGET): $(OBJS)
	gcc $(ARGS) $(OBJS) -o $(TARGET)

%.o: %.c
	gcc $(ARGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
