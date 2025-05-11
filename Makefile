# Компилятор
CC = gcc

# Флаги компиляции
CFLAGS = -I. -g -Wall -Wextra -std=c11 -DMG_ENABLE_LINES=1 -DMG_ENABLE_LOG=3

# Флаги линковщика (добавляем ws2_32 для Windows)
LDFLAGS = -pthread
ifeq ($(OS),Windows_NT)
    LDFLAGS += -lws2_32
endif

# Имя исполняемого файла
TARGET = server

# Исходные файлы
SOURCES = main.c mongoose.c input.c

# Правило по умолчанию
all: $(TARGET)

# Сборка основной программы
$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Очистка
clean:
	rm -f $(TARGET) *.o

# Специальные цели
.PHONY: all clean