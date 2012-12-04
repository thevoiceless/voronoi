OPENGL = -L/usr/X11R6/lib -L/usr/local/lib -lglut -lGLU -lGL -lm -lGLEW
OPT = -O2

OBJS = proj4.o
CPPS = proj4.cpp
EXES = proj4

all: proj4

proj4: objects $(OBJS)
	g++ $(OBJS) -o proj4 $(OPT) $(OPENGL)

objects: $(CPPS)
	g++ -c $(CPPS) $(OPT)

clean: 
	rm $(EXES)
	rm $(OBJS)

