# a little exercise

VERSION = 1.0

OBJ = 	main.o \
	mappedfile.o \
	yrequest.o \
	yprocessing.o

CC ?= gcc
CXX ?= g++

# Common compiler flags. See <http://blog.httrack.com/blog/2014/03/09/what-are-your-gcc-flags/> for
# a glance at most of them. Some flags are there for fancy reasons (such as -grecord-gcc-switches), others
# for security reasons (-fstack-protector-strong), and most ones for code correctness reasons.
# See also <https://wiki.debian.org/Hardening> for hardening hints
# We also use -march=native, as this is for this exercise, and not meant for production use.
COMMON_CFLAGS := \
	-D_FORTIFY_SOURCE=2 \
	-march=native \
	-pipe \
	-O3 \
	-g3 \
	-fPIC \
	-fno-common \
	-fstack-protector \
	-fstack-protector-strong \
	-fvisibility=hidden \
	-pthread \
	-grecord-gcc-switches \
	-Werror \
	-Wall \
	-Wextra \
	-Wbool-compare \
	-Wcast-align \
	-Wformat=2 \
	-Wformat-nonliteral \
	-Wformat-security \
	-Winit-self \
	-Wlogical-op \
	-Wmissing-format-attribute  \
	-Wno-unused-parameter \
	-Wpointer-arith  \
	-Wtrampolines \
	-Wundef \
	-Wuninitialized \
	-Wunused \
	-Wwrite-strings

# Common linker flags
COMMON_LDFLAGS := \
	-rdynamic \
	-Wl,-O1 \
	-Wl,--no-undefined \
	-Wl,--build-id=sha1 \
	-Wl,-z,relro \
	-Wl,-z,now \
	-Wl,-z,noexecstack

# Aimed for C (executable); -Wc++-compat is an attempt to relieve incurable C flaws
# (unused in this project)
CFLAGS ?= \
	-std=c99 \
	-fPIE \
	$(COMMON_CFLAGS) \
	-Wstrict-prototypes \
	-Wc++-compat

# Aimed for C++ only
CXXFLAGS ?= \
	-std=c++11 \
	-fno-exceptions \
	-fPIE \
	$(COMMON_CFLAGS)

# Aimed for link (executable)
LDFLAGS ?= \
	$(COMMON_LDFLAGS) \
	-pie

EXECFLAGS ?= 

LIBS ?= -lstdc++

INSTALL = install
INSTALL_DATA ?= $(INSTALL) -m644
INSTALL_PROGRAM ?= $(INSTALL) -m755
MKDIR ?= mkdir -p -m 755
TAR ?= tar
GROFF ?= groff
BASH = bash -c
VALGRIND ?= valgrind --quiet --track-origins=yes --leak-check=full

PREFIX ?= /usr
bindir ?= ${PREFIX}/bin/
man1dir ?= ${PREFIX}/share/man/man1
htmldir ?= ${PREFIX}/share/hnStat/html
pdfdir ?= ${PREFIX}/share/hnStat/pdf
man1ext ?= 1

all: build man tests cleanobjs

clean: cleanobjs
	rm -f *.so* *.dll hnStat hnStat.html hnStat.pdf

cleanobjs:
	rm -f *.o *.obj

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

%.o: %.cpp
	$(CXX) -c -o $@ $< $(CXXFLAGS)

build: hnStat
	
hnStat: $(OBJ)
	$(CC) -o $@ $^ $(CXXFLAGS) $(LDFLAGS) $(EXECFLAGS) $(LIBS)

tests:
	$(BASH) ./test-suite.sh

valgrind: build
	$(VALGRIND) --quiet --track-origins=yes --leak-check=full ./hnStat top 100 hn_logs.tsv >/dev/null

man:
	$(GROFF) -man -Thtml hnStat.1 > hnStat.html
	$(GROFF) -man -Tps hnStat.1 | ps2pdf - hnStat.pdf

dist:
	$(TAR) cfz hnStat_$(VERSION).orig.tar.gz *.c *.h *.sh *.1 *.txt Makefile

install: build man
	$(MKDIR) $(DESTDIR)$(bindir)
	$(MKDIR) $(DESTDIR)$(man1dir)
	$(MKDIR) $(DESTDIR)$(htmldir)
	$(MKDIR) $(DESTDIR)$(pdfdir)
	$(INSTALL_PROGRAM) hnStat $(DESTDIR)$(bindir)/
	$(INSTALL_DATA) hnStat.1 $(DESTDIR)$(man1dir)/hnStat.$(man1ext)
	$(INSTALL_DATA) hnStat.html $(DESTDIR)$(htmldir)/
	$(INSTALL_DATA) hnStat.pdf $(DESTDIR)$(pdfdir)/
