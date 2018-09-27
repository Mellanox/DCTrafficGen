MODE ?= debug
GCC_INSTALL ?= /auto/sw_tools/OpenSource/gcc/INSTALLS/gcc-4.9.3/linux_x86_64
export PATH:=${GCC_INSTALL}/bin:${PATH}

ifeq (${MODE},debug)
DEF=-DDEBUG
LIBSUFF=_debug
else
DEF=
LIBSUFF=
endif

CXXFLAGS="-Wall --std=c++11 -I /usr/include/libxml2/ ${DEF}"
all: checkmakefiles
	cd src && $(MAKE) CXXFLAGS=$(CXXFLAGS) CONFIGNAME= PROJECTRELATIVE_PATH=

clean: checkmakefiles
	cd src && $(MAKE) clean

cleanall: checkmakefiles
	cd src && $(MAKE) MODE=release clean
	cd src && $(MAKE) MODE=debug clean
	rm -f src/Makefile

makefiles:
	cd src && opp_makemake -P ${PWD} -M ${MODE} -f --deep --make-so -L${GCC_INSTALL}/lib64 -O lib -o DCTG_$$\(MODE\)

checkmakefiles:
	@if [ ! -f src/Makefile ]; then \
	echo; \
	echo '======================================================================='; \
	echo 'src/Makefile does not exist. Please use "make makefiles" to generate it!'; \
	echo '======================================================================='; \
	echo; \
	exit 1; \
	fi
