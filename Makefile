CXX = em++
CXXFLAGS = -Wall -O3 -sASYNCIFY -flto -fno-rtti -fno-exceptions --closure 1 -sENVIRONMENT=web,worker -s MODULARIZE=1 -s 'EXPORT_NAME="createStarfield"' -sALLOW_MEMORY_GROWTH

PYTHON = python3
PYTHON_ARGS = -m http.server

SOURCES = main.cpp 

MAIN = emout

$(MAIN): 
	$(CXX) $(CXXFLAGS) $(SOURCES) --shell-file resources/shell.html -o dist/$(MAIN).html

clean:
	$(RM) dist/*
base64: $(MAIN)
	base64 dist/emout.wasm > dist/emout.b64

start: clean $(MAIN)
	emrun dist/$(MAIN).html
