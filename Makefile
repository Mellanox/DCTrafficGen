# By default we build debug
MODE ?= debug

# If you provide an alternate GCC_INSTALL dir we take care of the PATH and LIBPATH 
# e.g. GCC_INSTALL ?= /auto/sw_tools/OpenSource/gcc/INSTALLS/gcc-4.9.3/linux_x86_64

# NOTICE: 
#   omnetpp-4.6 will build same lib name in release/debug. 
#   omnetpp-5* will suffic debug lib with _dbg

ifneq ($(GCC_INSTALL),) 
	export PATH:=${GCC_INSTALL}/bin:${PATH}
	GCC_LIB=-L${GCC_INSTALL}/lib64
endif

ifeq (${MODE},debug)
DEF=-DDEBUG -ggdb -O0
#LIBSUFF=debug
else
DEF=
#LIBSUFF=release
endif

CXXFLAGS="-Wall --std=c++11 -I /usr/include/libxml2/ ${DEF}"
all: checkmakefiles
	cd src && $(MAKE) MODE=$(MODE) CXXFLAGS=$(CXXFLAGS) 

clean: checkmakefiles
	cd src && $(MAKE) clean

cleanall: checkmakefiles
	cd src && $(MAKE) MODE=release clean
	cd src && $(MAKE) MODE=debug clean
	rm -f src/Makefile

makefiles:
	cd src && opp_makemake -P ${PWD} -f -X example --deep --make-so $(GCC_LIB) -O out -o DCTG

checkmakefiles:
	@if [ ! -f src/Makefile ]; then \
	echo; \
	echo '======================================================================='; \
	echo 'src/Makefile does not exist. Please use "make makefiles" to generate it!'; \
	echo '======================================================================='; \
	echo; \
	exit 1; \
	fi
