
CC ?= cc

TOPDIR = .
SRCDIR = src
BUILDDIR = build
CFLAGS = -MMD -I$(SRCDIR)
LDFLAGS = -L$(BUILDDIR)

all: laf laugh_test
clean:
	rm -rvf $(BUILDDIR)
	@echo '# reset makefile'
	@echo '' >Makefile
	@echo 'Makefile: project.yml' >>Makefile
	@echo '	tools/genmk.py' >>Makefile
	touch project.yml
.PHONY: all clean

Makefile: $(TOPDIR)/project.yml $(TOPDIR)/conf.yml $(TOPDIR)/tools/genmk.py
	$(TOPDIR)/tools/genmk.py

laf_OUTNAME = liblaf.so
laf_OUT = $(BUILDDIR)/$(laf_OUTNAME)
laf_DEP = 
laf_DEPFILES = 
laf_OUTDIR = build/out/laf
laf_SRCDIR = src/laf
laf_FILES = util class
laf_C = $(laf_FILES:%=$(laf_SRCDIR)/%.c)
laf_O = $(laf_FILES:%=$(laf_OUTDIR)/%.o)
laf_D = $(laf_O:.o=.d)
laf_CFLAGS = -fPIC -g
laf_LDFLAGS = -shared
laf_LDLIBS = -llua

laf: laf_DIRS $(laf_DEP) $(laf_OUT)
laf_DIRS:
	mkdir -pv $(BUILDDIR) $(laf_OUTDIR)
.PHONY: laf laf_DIRS

$(laf_OUT): $(laf_O) $(laf_DEPFILES)
	$(CC) $(laf_LDFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS) $(laf_LDLIBS)

$(laf_OUTDIR)/%.o: $(laf_SRCDIR)/%.c
	$(CC) $(laf_CFLAGS) $(CFLAGS) -c -o $@ $<

-include $(laf_D)

laf: $(laf_OUT)
.PHONY: laf

laugh_test_OUTNAME = laugh_test
laugh_test_OUT = $(BUILDDIR)/$(laugh_test_OUTNAME)
laugh_test_DEP = laf
laugh_test_DEPFILES = $(laf_OUT)
laugh_test_OUTDIR = build/out/laugh_test
laugh_test_SRCDIR = src/laugh_test
laugh_test_FILES = main foo
laugh_test_C = $(laugh_test_FILES:%=$(laugh_test_SRCDIR)/%.c)
laugh_test_O = $(laugh_test_FILES:%=$(laugh_test_OUTDIR)/%.o)
laugh_test_D = $(laugh_test_O:.o=.d)
laugh_test_CFLAGS = -g
laugh_test_LDFLAGS = 
laugh_test_LDLIBS = -llua

laugh_test: laugh_test_DIRS $(laugh_test_DEP) $(laugh_test_OUT)
laugh_test_DIRS:
	mkdir -pv $(BUILDDIR) $(laugh_test_OUTDIR)
.PHONY: laugh_test laugh_test_DIRS

$(laugh_test_OUT): $(laugh_test_O) $(laugh_test_DEPFILES)
	$(CC) $(laugh_test_LDFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS) $(laugh_test_LDLIBS)

$(laugh_test_OUTDIR)/%.o: $(laugh_test_SRCDIR)/%.c
	$(CC) $(laugh_test_CFLAGS) $(CFLAGS) -c -o $@ $<

-include $(laugh_test_D)
