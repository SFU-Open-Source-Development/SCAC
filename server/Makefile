BUILDDIR = src

OUTFILE = epollServer
OUTDIR = bin

CC = g++
CFLAGS = -Wall -Werror -std=c++2a -pthread
LFLAGS = sqlite3

app:
	mkdir -p bin
	mkdir -p db
	$(CC) $(CFLAGS) $(BUILDDIR)/*.cpp -o $(OUTDIR)/$(OUTFILE) -l $(LFLAGS)
	ls -l

done:
	@echo "Finished building application."
