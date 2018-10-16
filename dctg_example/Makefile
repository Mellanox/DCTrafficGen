# Point to DCTG - may want to update
DCTG=$(PWD)/..

# By default we build debug
MODE ?= debug

# If you provide an alternate GCC_INSTALL dir we take care of the PATH and LIBPATH 
# e.g. GCC_INSTALL ?= /auto/sw_tools/OpenSource/gcc/INSTALLS/gcc-4.9.3/linux_x86_64

# NOTICE: 
#   omnetpp-4.6 will build same lib name in release/debug. 
#   omnetpp-5* will suffic debug lib with _dbg

# We must know omnet version to be able to link with correct lib...
# since in 4.6 same lib name and in 5.x suffic _dbg ...
# see how we both define the suffic to opp_makemake and pass it via the make
OMNET_REV=$(shell opp_run -h | head -2 | tail -1 | sed 's/.*Version: \(...\).*/\1/')
ifneq ($(OMNET_REV), 4.6)
	DBG_SUFFIX:="_dbg"
endif

ifneq ($(GCC_INSTALL),) 
	export PATH:=${GCC_INSTALL}/bin:${PATH}
	GCC_LIB=-L${GCC_INSTALL}/lib64
endif

CXXFLAGS="-Wall --std=c++11 -I/usr/include/libxml2/"
all: checkmakefiles
	cd src && $(MAKE) MODE=$(MODE) CXXFLAGS=$(CXXFLAGS) DBG_SUFFIX=$(DBG_SUFFIX)

clean: checkmakefiles
	cd src && $(MAKE) clean

cleanall: checkmakefiles
	cd src && $(MAKE) MODE=release clean
	cd src && $(MAKE) MODE=debug clean
	rm -f src/Makefile

makefiles:
	cd src && opp_makemake -P ${PWD} -f --deep -I$(DCTG)/src -L$(DCTG)/out/$$\(CONFIGNAME\)/src -lDCTG$$\(DBG_SUFFIX\) -O out -o dctg_example

checkmakefiles:
	@if [ ! -f src/Makefile ]; then \
	echo; \
	echo '======================================================================='; \
	echo 'src/Makefile does not exist. Please use "make makefiles" to generate it!'; \
	echo '======================================================================='; \
	echo; \
	exit 1; \
	fi
