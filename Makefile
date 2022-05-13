CXX = em++
CXXFLAGS = -Wall -O3 -sASYNCIFY

PYTHON = python3
PYTHON_ARGS = -m http.server

SOURCES = main.cpp 

MAIN = emout

$(MAIN): 
	$(CXX) $(CXXFLAGS) $(SOURCES) -o dist/$(MAIN).html

clean:
	$(RM) dist/*

start: clean $(MAIN)
	emrun dist/$(MAIN).html
