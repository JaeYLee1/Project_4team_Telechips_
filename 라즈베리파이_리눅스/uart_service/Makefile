CC = gcc
CFLAGS = -Wall -Wextra -Werror -O2 -std=c11

TARGET = pbv_uart_service
SRC = uart_service.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)
