BUILDDIR = src

OUTFILE = epollServer
OUTDIR = bin

CC = g++
CFLAGS = -Wall -Werror -std=c++2a -pthread

app:
	mkdir -p bin
	$(CC) $(CFLAGS) $(BUILDDIR)/* -o $(OUTDIR)/$(OUTFILE)
	ls -l

done:
	@echo "Finished building application."
