# To be called in fodge/src

PROG = ../fodge

IDIR = ../include
CC = gcc
CFLAGS = -Wall -Werror -Wextra -std=c99 -I$(IDIR)

ODIR = obj

# Files that directly implement a .h file
IMPFILES =  $(ODIR)/cycrep.o $(ODIR)/diagram_table.o $(ODIR)/fodge_FORM.o $(ODIR)/fodge_TikZ.o $(ODIR)/fodge_util.o $(ODIR)/fsp_map.o $(ODIR)/info_sort.o $(ODIR)/point.o
# Additional files with no .h counterpart
EXTFILES = $(ODIR)/diagram.o $(ODIR)/polygon.o $(ODIR)/represent.o $(ODIR)/fodge_main.o

OBJFILES = $(IMPFILES) $(EXTFILES)

# Standard dependencies
$(IMPFILES) : $(patsubst $(ODIR)/%.o,$(IDIR)/%.h,$@)
$(OBJFILES) : $(IDIR)/fodge.h $(IDIR)/fodge_util.h $(IDIR)/cycrep.h

# Additional dependencies
$(ODIR)/fodge_main.o : $(IDIR)/fodge_TikZ.h $(IDIR)/fodge_FORM.h
$(ODIR)/diagram.o    : $(IDIR)/fsp_map.h
$(ODIR)/fodge_FORM.o : $(IDIR)/fsp_map.h
$(ODIR)/fodge_TikZ.o : $(IDIR)/point.h
$(ODIR)/cycrep.o     : $(IDIR)/info_sort.h

$(IMPFILES) $(EXTFILES): $(patsubst $(ODIR)/%.o,%.c,$@)
	mkdir -p $(ODIR)
	rm -f "$@.d"
	$(CC) -c -o $@ $(patsubst $(ODIR)/%.o,%.c,$@) $(CFLAGS) -MMD -MP -MF "$@.d"

fodge: $(IMPFILES) $(EXTFILES)
	$(CC) -o $(PROG) $^ $(CFLAGS) -lm

.PHONY: clean

clean:
	rm -f $(PROG) $(ODIR)/*.o $(ODIR)/*.o.d
