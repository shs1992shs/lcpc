#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux
CND_DLIB_EXT=so
CND_CONF=Release
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/lib/clipper/cpp/clipper.o \
	${OBJECTDIR}/lib/common/shapes.o \
	${OBJECTDIR}/lib/sweep/advancing_front.o \
	${OBJECTDIR}/lib/sweep/cdt.o \
	${OBJECTDIR}/lib/sweep/sweep.o \
	${OBJECTDIR}/lib/sweep/sweep_context.o \
	${OBJECTDIR}/src/coords.o \
	${OBJECTDIR}/src/funnel.o \
	${OBJECTDIR}/src/lcpfinder.o \
	${OBJECTDIR}/src/main.o


# C Compiler Flags
CFLAGS=-pg -f

# CC Compiler Flags
CCFLAGS=-l gdal -pg -fpermissive
CXXFLAGS=-l gdal -pg -fpermissive

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-L/usr/include/gdal -lgdal `pkg-config --libs python-2.7`  

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/lcpc

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/lcpc: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/lcpc ${OBJECTFILES} ${LDLIBSOPTIONS} -lgdal -pg

${OBJECTDIR}/lib/clipper/cpp/clipper.o: lib/clipper/cpp/clipper.cpp
	${MKDIR} -p ${OBJECTDIR}/lib/clipper/cpp
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -w -I/usr/include/gdal `pkg-config --cflags python-2.7` -std=c++14  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/lib/clipper/cpp/clipper.o lib/clipper/cpp/clipper.cpp

${OBJECTDIR}/lib/common/shapes.o: lib/common/shapes.cc
	${MKDIR} -p ${OBJECTDIR}/lib/common
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -w -I/usr/include/gdal `pkg-config --cflags python-2.7` -std=c++14  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/lib/common/shapes.o lib/common/shapes.cc

${OBJECTDIR}/lib/sweep/advancing_front.o: lib/sweep/advancing_front.cc
	${MKDIR} -p ${OBJECTDIR}/lib/sweep
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -w -I/usr/include/gdal `pkg-config --cflags python-2.7` -std=c++14  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/lib/sweep/advancing_front.o lib/sweep/advancing_front.cc

${OBJECTDIR}/lib/sweep/cdt.o: lib/sweep/cdt.cc
	${MKDIR} -p ${OBJECTDIR}/lib/sweep
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -w -I/usr/include/gdal `pkg-config --cflags python-2.7` -std=c++14  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/lib/sweep/cdt.o lib/sweep/cdt.cc

${OBJECTDIR}/lib/sweep/sweep.o: lib/sweep/sweep.cc
	${MKDIR} -p ${OBJECTDIR}/lib/sweep
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -w -I/usr/include/gdal `pkg-config --cflags python-2.7` -std=c++14  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/lib/sweep/sweep.o lib/sweep/sweep.cc

${OBJECTDIR}/lib/sweep/sweep_context.o: lib/sweep/sweep_context.cc
	${MKDIR} -p ${OBJECTDIR}/lib/sweep
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -w -I/usr/include/gdal `pkg-config --cflags python-2.7` -std=c++14  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/lib/sweep/sweep_context.o lib/sweep/sweep_context.cc

${OBJECTDIR}/src/coords.o: src/coords.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -w -I/usr/include/gdal `pkg-config --cflags python-2.7` -std=c++14  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/coords.o src/coords.cpp

${OBJECTDIR}/src/funnel.o: src/funnel.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -w -I/usr/include/gdal `pkg-config --cflags python-2.7` -std=c++14  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/funnel.o src/funnel.cpp

${OBJECTDIR}/src/lcpfinder.o: src/lcpfinder.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -w -I/usr/include/gdal `pkg-config --cflags python-2.7` -std=c++14  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/lcpfinder.o src/lcpfinder.cpp

${OBJECTDIR}/src/main.o: src/main.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -w -I/usr/include/gdal `pkg-config --cflags python-2.7` -std=c++14  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/main.o src/main.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
