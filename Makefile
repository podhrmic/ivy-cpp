
# $Id: Makefile 3625 2014-12-09 14:28:33Z bustico $

os=$(shell uname -s)
PATH := $(PATH):/usr/lib/qt4/bin/:/usr/share/qt4/bin/

ifeq ($(os),Darwin)
    IVY_PATH := $(HOME)/src-ext/ivy-c-3.6/src
    CPPFLAGS = -MMD  -I$(IVY_PATH)
    DSO_EXT = .dylib
    LD = g++ -dynamiclib -single_module
    LDFLAGS = -L$(IVY_PATH) -L/sw/lib -framework OpenGL -framework AGL -framework Carbon
else
    PERHAPS64 := $(shell uname --machine | perl -ne "print /64/ ? '64' : '';")
    LIB:= lib$(PERHAPS64) #-pthread -L/usr/$(LIB)

    CPPFLAGS = -MMD -I/usr/X11R6/include -I/usr/local/include/Ivy
    DSO_EXT = .so
    LD = g++ -shared -fPIC
    #PREFIX=/usr/local
    ifndef PREFIX
        PREFIX=/usr
        LDFLAGS = -L$(PREFIX)/$(LIB)
    else
        LDFLAGS = -L$(PREFIX)/$(LIB)/ivy
    endif
endif

CASESENSITIVE = "yes"
ifeq ($(CASESENSITIVE), "no")
	PCRE_OPT=PCRE_CASELESS
	REGCOMP_OPT=REG_ICASE
else
	PCRE_OPT=0
	REGCOMP_OPT=REG_EXTENDED	
endif
REGEXP= -DUSE_PCRE_REGEX -DPCRE_OPT=$(PCRE_OPT)
PCREINC = $(shell pcre-config --cflags)


CXXFILES := Ivy.cxx IvyApplication.cxx
OBJECTS := $(CXXFILES:.cxx=.o)
DEPS := $(CXXFILES:.cxx=.d)

LD_QUICKSTART_INFO=
GCXXINCS= 

LIBIVY = libIvy

LIBIVY_STATIC = $(LIBIVY).a
LIBIVY_SHARED = $(LIBIVY)$(DSO_EXT)
LIBIVY_DEPLIBS = -livy

ifdef DEBUG
    CC= g++ -fPIC -g -Wall -pg $(CPPFLAGS)
    DBG=debug
else
    CC= g++ -fPIC -O2  -Wall $(CPPFLAGS)
    DBG=
endif

default: $(LIBIVY_STATIC) $(LIBIVY_SHARED) \
	 testIvy

%.o: %.cxx
	$(CC) -c $<

$(LIBIVY_STATIC) : $(OBJECTS)
	ar rv $(LIBIVY_STATIC) $(OBJECTS)
	ranlib $(LIBIVY_STATIC)

$(LIBIVY_SHARED) : $(OBJECTS)
	$(LD) -o $(LIBIVY_SHARED) $(OBJECTS) $(LDFLAGS) $(LIBIVY_DEPLIBS)

install: default
    # headers
	mkdir -p $(DESTDIR)$(PREFIX)/include/Ivy
	install -m 0644 Ivy*.h $(DESTDIR)$(PREFIX)/include/Ivy
	# libs
	mkdir -p $(DESTDIR)$(PREFIX)/$(LIB)
	install -m 0644 $(LIBIVY_STATIC) $(DESTDIR)$(PREFIX)/$(LIB)
	install -m 0644 $(LIBIVY_SHARED) $(DESTDIR)$(PREFIX)/$(LIB)

LLDLIBS = -L. -Wl,-rpath,. -L$(IVY_PATH)/src -Wl,-rpath,/usr/local/lib64 -L/usr/local/lib64

testIvy : testIvy.cxx
	g++  -g  $(CPPFLAGS) $(LLDLIBS) -std=c++11 -o $@  testIvy.cxx -lIvy -pthread


distclean clean : 
	rm -f $(LIBIVY_STATIC) $(LIBIVY_SHARED) \
		$(OBJECTS) $(DEPS) \
		core *.o *.d *~ *.moc testIvy

-include *.d

