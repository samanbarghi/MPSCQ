SRC_DIR := 	tests
BIN_DIR :=	bin

CXX 	:=	g++  -std=c++1y
RM 		:= 	rm -rf

SRCEXT 	:= cpp
SOURCES := $(shell find $(SRC_DIR) -type f -name *.$(SRCEXT))
OBJECTS := $(patsubst $(SRC_DIR)/%,$(BIN_DIR)/%,$(SOURCES:.$(SRCEXT)=.o))

CXXFLAGS:=	-g -O2
INC 	:= -I src

all: $(OBJECTS)

$(BIN_DIR)/%.o: $(SRC_DIR)/%.$(SRCEXT)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(INC) -o $@ $< -pthread

clean:
	$(RM) $(BIN_DIR)