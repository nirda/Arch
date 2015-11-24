FLAGS=-w
CC=g++
MESH_INC=-I/usr/local/include/OpenMesh/Core
MESH_LIB=-L/usr/local/lib -lOpenMeshCore
CFLAGS=$(FLAGS) $(MESH_INC)
LIBS=-lglut -lGL -lGLU -lm $(MESH_LIB)
All:stone1
stone1:
	$(CC) $(CFLAGS) *.cpp -o stoneView $< $(LIBS)
#CC = g++ -Wall -Wno-deprecated
#OpenMesh definitions
#CG_HOME = /cs/course/current/cg
#OM_DIR = $(CG_HOME)/OpenMesh
#OM_LIBS = -L$(OM_DIR)/lib -lOpenMeshCore
#OM_INCLUDE=  -I$(OM_DIR)/include

#OpenMesh definitions
#CG_HOME = /cs/course/2013/cg
#OM_DIR = $(CG_HOME)/OpenMesh
#OM_LIBS = -L$(OM_DIR)/lib -lOpenMeshCore
#OM_INCLUDE=  -I$(OM_DIR)/include

#LINK_FLAGS = -L/usr/lib -lm -lglut -lGL -lGLU -ldl -L/usr/X11R6/lib $(OM_LIBS)
#COMP_FLAGS = -O2 -I/usr/include -I/usr/X11R6/include $(OM_INCLUDE)

#all: ex1

#ex1: *.cpp
#	$(CC) $(COMP_FLAGS) $(LINK_FLAGS) *.cpp -o ex1

#clean:
#	/bin/rm -f *.o *.bak ex1 *core *~
