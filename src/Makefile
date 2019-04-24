IDIR = ../include
CC = gcc
CFLAGS = -Wall -Werror -Wextra -std=c99 -I$(IDIR)

ODIR = obj

# Files that directly implement a .h file
IMPFILES = $(pathsubst %,$(ODIR),cycrep.o diagram_table.o fodge_FORM.o fodge_TikZ.o fodge_util.o fsp_map.o info_sort.o point.o)
# Additional files with no .h counterpart
EXTFILES = $(pathsubst %,$(ODIR),diagram.o polygon.o represent.o fodge_main.o)

OBJFILES = $(IMPFILES) $(EXTFILES)

# Standard dependencies
$(IMPFILES) : $(subst .o,.h,$@)
$(OBJFILES) : fodge.h fodge_util.h

# Additional dependencies
represent.o : cycrep.h
fodge_main.o : fodge_TikZ.h fodge_FORM.h
diagram.o : fsp_map.h
fodge_TikZ.h : point.h

$(ODIR)/%.o: %.c
	$(CC) -c -c $@ $< $(CFLAGS)
	
fodgemake: $(OBJFILES)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm $(ODIR)/*.o
