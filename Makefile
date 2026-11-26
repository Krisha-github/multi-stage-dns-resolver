CXX := g++
CXXFLAGS := -std=c++11 -Wall -Wextra -Wpedantic -O2
WINLIB :=
ifeq ($(OS),Windows_NT)
WINLIB := -lws2_32
RM := del /Q /F
EXE := .exe
else
RM := rm -f
EXE :=
endif

COMMON := socket_utils.cpp input_utils.cpp

all: client_app$(EXE) proxy_app$(EXE) dns_app$(EXE)

client_app$(EXE): client_app.cpp $(COMMON) socket_utils.h input_utils.h
	$(CXX) $(CXXFLAGS) -o $@ client_app.cpp $(COMMON) $(WINLIB)

proxy_app$(EXE): proxy_app.cpp $(COMMON) socket_utils.h input_utils.h
	$(CXX) $(CXXFLAGS) -o $@ proxy_app.cpp $(COMMON) $(WINLIB)

dns_app$(EXE): dns_app.cpp $(COMMON) socket_utils.h input_utils.h
	$(CXX) $(CXXFLAGS) -o $@ dns_app.cpp $(COMMON) $(WINLIB)

clean:
	$(RM) client_app$(EXE) proxy_app$(EXE) dns_app$(EXE)
