#################################
### USER CUSTOMIZABLE SECTION ###
#################################

### SET OPTIONAL #defines HERE  ###

### Don't remove the line below ###
DEFINITIONS=
###

## Uncomment (Comment) to (not) export "raw" modules for viewing in ROOT treew
DEFINITIONS+=-DDISPLAY_MODULES

## (Un)comment to (not) ignore corresponding heavy-ion detectors
#DEFINITIONS+=DRAGON_OMIT_DSSSD
#DEFINITIONS+=DRAGON_OMIT_IC
#DEFINITIONS+=DRAGON_OMIT_NAI
#DEFINITIONS+=DRAGON_OMIT_GE


### Set to YES (NO) to turn on (off) root [or rootbeer, or rootana, or ...] usage ###
USE_ROOT=YES
## USE_ROOTANA=YES
USE_ROOTBEER=YES

## Automatically turn off rootana if on jabberwock
THE_HOST := $(shell hostname)
ifeq ($(THE_HOST),jabberwock.triumf.ca)
USE_ROOTANA=NO
endif


### Set ROOTBEER home directory (ignore if USE_ROOTBEER=NO) ###
RB_HOME=$(HOME)/packages/rootbeer


### CHOOSE YOUR COMPILER IF YOU WANT ###
##CXX=g++ -Wall
#CXX=clang++ -I/opt/local/include/

##CC=gcc -Wall
#CC=clang -I/opt/local/include/


#####################################
### END USER CUSTOMIZABLE SECTION ###
#####################################


### Variable definitions
SRC=$(PWD)/src
OBJ=$(PWD)/obj
CINT=$(PWD)/cint
DRLIB=$(PWD)/lib

DYLIB=-shared
FPIC=-fPIC
INCFLAGS=-I$(SRC) -I$(CINT)
DEBUG=-ggdb -O3 -DDEBUG
CXXFLAGS=$(DEBUG) $(INCFLAGS) $(DEFINITIONS)


ROOTLIBS=
ifeq ($(USE_ROOT),YES)
DEFINITIONS+= -DUSE_ROOT
ifdef ROOTSYS
ROOTLIBS= -L$(ROOTSYS)/lib $(shell $(ROOTSYS)/bin/root-config --cflags --libs --glibs) -I$(ROOTSYS)/include -lXMLParser
CXXFLAGS += -I$(ROOTSYS)/include
else
ROOTLIBS= - $(shell $(ROOTSYS)/bin/root-config --cflags --libs --glibs) -lXMLParser -lThread -lTreePlayer
endif
else
USE_ROOTBEER=NO
endif

ifdef MIDASSYS
CXXFLAGS += -DMIDASSYS
MIDASLIBS = -lmidas -L$(MIDAS_LIB_DIR)
INCFLAGS += -I$(MIDASSYS)/include
endif


UNAME=$(shell uname)
ifeq ($(UNAME),Darwin)
ifdef MIDASSYS
CXXFLAGS += -DOS_LINUX -DOS_DARWIN
MIDAS_LIB_DIR=$(MIDASSYS)/darwin/lib
endif
DYLIB=-dynamiclib -single_module -undefined dynamic_lookup
FPIC=
RPATH=
endif

ifeq ($(UNAME),Linux)
ifdef MIDASSYS
CXXFLAGS += -DOS_LINUX
MIDAS_LIB_DIR=$(MIDASSYS)/linux/lib
MIDASLIBS+= -lm -lz -lutil -lnsl -lrt
endif
endif



CXX+=$(CXXFLAGS)

CC+=$(CXXFLAGS)

LINK=$(CXX) $(ROOTLIBS) $(RPATH) -L$(PWD)/lib

MAKE_DRAGON_DICT=
DR_DICT=
DR_DICT_DEP=
ifeq ($(USE_ROOT),YES)
MAKE_DRAGON_DICT+=rootcint -f $@ -c $(CXXFLAGS) -p $(HEADERS) TTree.h $(CINT)/Linkdef.h
DR_DICT=$(CINT)/DragonDictionary.cxx 
DR_DICT_DEP=$(CINT)/DragonDictionary.cxx 
endif


#### DRAGON LIBRARY ####
OBJECTS=                            		\
$(OBJ)/midas/mxml.o  	    	  		\
$(OBJ)/midas/strlcpy.o			     	\
$(OBJ)/midas/Odb.o                  		\
$(OBJ)/midas/Xml.o                  		\
$(OBJ)/midas/libMidasInterface/TMidasFile.o  	\
$(OBJ)/midas/libMidasInterface/TMidasEvent.o 	\
$(OBJ)/midas/libMidasInterface/TMidasOnline.o 	\
$(OBJ)/midas/Event.o                	\
					\
$(OBJ)/utils/Banks.o			\
$(OBJ)/utils/Unpack.o			\
					\
$(OBJ)/TStamp.o		              	\
$(OBJ)/Vme.o				\
$(OBJ)/Dragon.o					
## END OBJECTS ##


HEADERS=	                	\
$(SRC)/midas/*.hxx	        	\
$(SRC)/midas/*.h			\
$(SRC)/midas/libMidasInterface/*.h     	\
$(SRC)/utils/*.hxx        		\
$(SRC)/utils/*.h          		\
$(SRC)/*.hxx


### DRAGON LIBRARY ###
MAKE_ALL=$(DRLIB)/libDragon.so $(PWD)/bin/mid2root
ifeq ($(USE_ROOTBEER),YES)
MAKE_ALL+=$(PWD)/bin/rbdragon
## MAKE_ALL+=$(DRLIB)/libRBDragon.so
endif
ifeq ($(USE_ROOTANA),YES)
MAKE_ALL+=anaDragon
endif

all:  $(MAKE_ALL)

libDragon: $(DRLIB)/libDragon.so

$(DRLIB)/libDragon.so: $(DR_DICT_DEP) $(OBJECTS)
	$(LINK) $(DYLIB) $(FPIC) $(MIDASLIBS) \
\
$(OBJECTS) $(DR_DICT) \
\
-o $@ \

$(PWD)/bin/mid2root: src/mid2root.cxx $(DRLIB)/libDragon.so
	$(LINK) -lDragon $< \
-o $@ \

mid2root: $(PWD)/bin/mid2root

RbDragon.o: $(OBJ)/rootbeer/RbDragon.o

### OBJECT FILES ###

$(OBJ)/rootbeer/%.o: $(SRC)/rootbeer/%.cxx $(SRC)/rootbeer/*.hxx $(DR_DICT_DEP)
	$(CXX) $(RB_DEFS) $(RBINC) $(FPIC) -c \
-o $@ $< \

$(OBJ)/utils/%.o: $(SRC)/utils/%.cxx $(DR_DICT_DEP)
	$(CXX) $(FPIC) -c \
-o $@ $< \

$(OBJ)/midas/%.o: $(SRC)/midas/%.c $(DR_DICT_DEP)
	$(CC) $(FPIC) -c \
-o $@ $< \

$(OBJ)/midas/libMidasInterface/%.o: $(SRC)/midas/libMidasInterface/%.cxx $(DR_DICT_DEP)
	$(CXX) $(FPIC) -c \
-o $@ $< \

$(OBJ)/midas/%.o: $(SRC)/midas/%.cxx $(DR_DICT_DEP)
	$(CXX) $(FPIC) -c \
-o $@ $< \

$(OBJ)/rootana/%.o: $(SRC)/rootana/%.cxx $(CINT)/rootana/Dict.cxx
	$(CXX) $(ROOTANA_FLAGS) $(ROOTANA_DEFS) -c $(FPIC) \
-o $@ $< $(ROOTLIBS) \

## Must be last object rule!!
$(OBJ)/%.o: $(SRC)/%.cxx $(DR_DICT_DEP)
	$(CXX) $(FPIC) -c \
-o $@ $< \

### CINT DICTIONARY ###
dict: $(CINT)/DragonDictionary.cxx

$(CINT)/DragonDictionary.cxx:  $(HEADERS) $(CINT)/Linkdef.h
	$(MAKE_DRAGON_DICT) \

definitions:
	scp dragon@ladd06.triumf.ca:/home/dragon/online/src/definitions.h \
$(PWD)/src/utils/


### FOR ROOTANA ###
ROOTANA=$(HOME)/packages/rootana
ROOTANA_FLAGS=-ansi -Df2cFortran -I$(ROOTANA)
ROOTANA_DEFS=-DROOTANA_DEFAULT_HISTOS=$(PWD)/histos.dat

ROOTANA_REMOTE_OBJS=				\
$(ROOTANA)/libNetDirectory/netDirectoryServer.o

ROOTANA_OBJS=					\
$(OBJ)/rootana/Application.o			\
$(OBJ)/rootana/Callbacks.o			\
$(OBJ)/rootana/HistParser.o			\
$(OBJ)/rootana/Directory.o

ROOTANA_HEADERS= $(SRC)/rootana/Globals.h $(SRC)/rootana/*.hxx

ROOTANA_LIBS=-lrootana -lNetDirectory -L/home/dragon/packages/rootana/libNetDirectory/ -L/home/dragon/packages/rootana/

$(CINT)/rootana/Dict.cxx: $(ROOTANA_HEADERS) $(SRC)/rootana/Linkdef.h $(CINT)/DragonDictionary.cxx
	rootcint -f $@ -c $(CXXFLAGS) $(ROOTANA_FLAGS) -p $(ROOTANA_HEADERS) $(SRC)/rootana/Linkdef.h \

$(CINT)/rootana/CutDict.cxx: $(SRC)/rootana/Cut.hxx $(SRC)/rootana/CutLinkdef.h
	rootcint -f $@ -c $(CXXFLAGS) $(ROOTANA_FLAGS) -p $(SRC)/rootana/Cut.hxx $(SRC)/rootana/CutLinkdef.h \

$(DRLIB)/libRootanaCut.so: $(CINT)/rootana/CutDict.cxx
	$(LINK)  $(DYLIB) $(FPIC) $(RBINC) $(ROOTANA_FLAGS) $(ROOTANA_DEFS)  \
-o $@ $< \

libRootanaDragon.so: $(DRLIB)/libDragon.so $(CINT)/rootana/Dict.cxx $(DRLIB)/libRootanaCut.so $(ROOTANA_OBJS)
	$(LINK)  $(DYLIB) $(FPIC) $(RBINC) $(ROOTANA_FLAGS) $(ROOTANA_DEFS)  \
-o $@ $< $(CINT)/rootana/Dict.cxx $(ROOTANA_OBJS) -lDragon -lRootanaCut -L$(DRLIB) \

$(PWD)/bin/anaDragon: $(SRC)/rootana/anaDragon.cxx $(DRLIB)/libDragon.so $(CINT)/rootana/Dict.cxx $(DRLIB)/libRootanaCut.so $(ROOTANA_OBJS) $(ROOTANA_REMOTE_OBJS)
	$(LINK)  $(RBINC) $(ROOTANA_FLAGS) $(ROOTANA_DEFS) \
-o $@ $< $(CINT)/rootana/Dict.cxx $(ROOTANA_OBJS) -lDragon -lRootanaCut -L$(DRLIB) $(ROOTANA_LIBS) $(MIDASLIBS) \

rootana_clean:
	rm -f $(ROOTANA_OBJS) anaDragon libRootanaDragon.so $(CINT)/rootana/* $(DRLIB)/libRootanaCut.so

Dragon: $(OBJ)/Dragon.o


### FOR ROOTBEER ###

RBINC=-I$(RB_HOME)/src
RB_OBJECTS= 				\
$(OBJ)/rootbeer/RbDragon.o
# $(OBJ)/rootbeer/Timestamp.o		\
# $(OBJ)/rootbeer/MidasBuffer.o		\
# $(OBJ)/rootbeer/DragonEvents.o		\
# $(OBJ)/rootbeer/DragonRootbeer.o	\

RB_HEADERS= $(SRC)/rootbeer/RbDragon.hxx
##$(SRC)/rootbeer/*.hxx

RB_DEFS=-DRB_DRAGON_HOMEDIR=$(PWD)

## $(DRLIB)/libRbDragon.so
$(PWD)/bin/rbdragon: $(RB_OBJECTS)
	$(LINK) $(RB_OBJECTS) -L$(PWD)/../../rootbeer/lib  -lDragon -lRootbeer -lRbMidas \
-o $@ \

$(DRLIB)/libRbDragon.so: $(RB_OBJECTS) $(RB_HEADERS)
	$(LINK) $(RB_DEFS) $(RBINC) $(DYLIB) $(FPIC) $(RB_OBJECTS) -L$(PWD)/../../rootbeer/lib -lRbMidas \
-o $@ \

libRbDragon: $(DRLIB)/libRbDragon.so



Timestamp: $(OBJ)/rootbeer/Timestamp.o
MidasBuffer: $(OBJ)/rootbeer/MidasBuffer.o
DragonEvents: $(OBJ)/rootbeer/DragonEvents.o
DragonRootbeer: $(OBJ)/rootbeer/DragonRootbeer.o



#### REMOVE EVERYTHING GENERATED BY MAKE ####

clean:
	rm -rf $(DRLIB)/*.so $(CINT)/DragonDictionary.* $(OBJECTS) $(RB_OBJECTS) obj/rootana/*.o $(CINT)/rootana/* bin/*


#### FOR DOXYGEN ####

doc::
	$(HOME)/bin/doxygen doc/Doxyfile

docclean::
	rm -fr /triumfcs/trshare/gchristian/public_html/dragon/analyzer/html \
/triumfcs/trshare/gchristian/public_html/dragon/analyzer/latex


#### FOR UNIT TESTING ####

mxml.o:           $(OBJ)/midas/libMidasInterface/mxml.o
strlcpy.o:        $(OBJ)/midas/libMidasInterface/strlcpy.o
Odb.o:            $(OBJ)/midas/Odb.o
Xml.o:            $(OBJ)/midas/Xml.o
TMidasFile.o:     $(OBJ)/midas/libMidasInterface/TMidasFile.o
TMidasEvent.o:    $(OBJ)/midas/libMidasInterface/TMidasEvent.o
Event.o:          $(OBJ)/midas/Event.o

TStamp.o:         $(OBJ)/tstamp/TStamp.o

V792.o:           $(OBJ)/vme/V792.o
V1190.o:          $(OBJ)/vme/V1190.o
Io32.o:           $(OBJ)/vme/Io32.o

Bgo.o:            $(OBJ)/dragon/Bgo.o
Mcp.o:            $(OBJ)/dragon/Mcp.o
Dsssd.o:          $(OBJ)/dragon/Dsssd.o
Auxillary.o:      $(OBJ)/dragon/Auxillary.o
IonChamber.o:     $(OBJ)/dragon/IonChamber.o
SurfaceBarrier.o: $(OBJ)/dragon/SurfaceBarrier.o

Head.o:           $(OBJ)/dragon/Head.o
Tail.o:           $(OBJ)/dragon/Tail.o
Scaler.o:         $(OBJ)/dragon/Scaler.o

test/%: test/%.cxx $(DRLIB)/libDragon.so
	$(LINK) \
$< -o $@ \
-DMIDASSYS -lDragon -L$(DRLIB) $(MIDASLIBS) -DODB_TEST -I$(PWD)/src


odbtest: $(DRLIB)/libDragon.so
	$(LINK) src/midas/Odb.cxx -o test/odbtest -DMIDASSYS -lDragon -L$(DRLIB) $(MIDASLIBS) -DODB_TEST -I$(PWD)/src

filltest: test/filltest.cxx $(DRLIB)/libDragon.so
	$(LINK) test/filltest.cxx -o test/filltest -DMIDAS_BUFFERS -lDragon -L$(DRLIB) -I$(PWD)/src \
