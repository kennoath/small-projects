#LDFLAGS = -lSDL2 -lSDL2_image -lrt -lasound -ljack -lpthread -lportaudio -lm
LDFLAGS = -lSDL2 -lSDL2_image -lGL -ldl -lm -lfreetype

INCLUDES = -I/usr/include/SDL2 -Iggl/ -I.
INCLUDES += -I/usr/include/freetype2

CFLAGS = -Wall -Werror -Wno-unused-variable -Wno-unused-const-variable -Wno-missing-braces -g -O3
SRCS = $(wildcard *.c)
SRCS += $(wildcard ggl/*.c)

nxplore: $(SRCS)
	clang $(SRCS) -o tree $(CFLAGS) $(INCLUDES) $(LDFLAGS)

.PHONY: clean
clean:
