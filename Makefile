MAINFILE := main
BINFILE := app
INCFOLDER := inc
SRCFOLDER := src
OBJFOLDER := obj

CC := gcc 
CFLAGS := -Wextra -Wall -pedantic
LFLAGS := -lwiringPi
SRCFILES := $(wildcard $(SRCFOLDER)/*.c)

all: $(SRCFILES:$(SRCFOLDER)/%.c=$(OBJFOLDER)/%.o)
	$(CC) $(CFLAGS) $(OBJFOLDER)/*.o -o $(BINFILE) $(LFLAGS)

obj/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@ -I./$(INCFOLDER) $(LFLAGS)
	$(CC) $(CFLAGS) -c $(MAINFILE).c -o $(OBJFOLDER)/$(MAINFILE).o $(LFLAGS)

run: $(BINFILE)
	./$(BINFILE)

.PHONY: clean
clean:
	rm -r ./${OBJFOLDER}/*
	rm -r ./$(BINFILE)
