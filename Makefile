# SYSCONF_LINK = g++
# CPPFLAGS     =
# LDFLAGS      =
# LIBS         = -lm

# DESTDIR = ./
# TARGET  = main

# OBJECTS := $(patsubst %.cpp,%.o,$(wildcard *.cpp))

# all: $(DESTDIR)$(TARGET)

# $(DESTDIR)$(TARGET): $(OBJECTS)
# 	$(SYSCONF_LINK) -Wall $(LDFLAGS) -o $(DESTDIR)$(TARGET) $(OBJECTS) $(LIBS)

# $(OBJECTS): %.o: %.cpp
# 	$(SYSCONF_LINK) -Wall $(CPPFLAGS) -c $(CFLAGS) $< -o $@

# clean:
# 	-rm -f $(OBJECTS)
# 	-rm -f $(TARGET)
# 	-rm -f *.tga

CXX = clang++
CXXFLAGS = -std=c++11 -Wall -Wextra
LDFLAGS = -framework OpenGL -framework GLUT
LIBS = -lm

DESTDIR = ./
TARGET = main

SOURCES = $(wildcard *.cpp)
OBJECTS = $(SOURCES:.cpp=.o)

all: $(DESTDIR)$(TARGET)

$(DESTDIR)$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(DESTDIR)$(TARGET) $(LDFLAGS) $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS)
	rm -f $(TARGET)
	rm -f *.tga