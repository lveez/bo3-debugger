PROJECT 			 	  = bo3dbg
BUILD_DIR  		   ?= build
BASE_BUILD_DIR   := $(BUILD_DIR)

BIN						 	  = $(BUILD_DIR)/$(PROJECT)
SOURCES 			    = src/process.cpp src/main.cpp
OBJS 					    = $(SOURCES:src/%.cpp=$(BUILD_DIR)/%.o)
DEPS					    = $(OBJS:.o=.d)

DEFAULT_CXXFLAGS 	= -Wall -Wextra -MMD -std=c++20
CXXFLAGS				 += $(DEFAULT_CXXFLAGS)

CXX						    = x86_64-w64-mingw32-g++




ifneq ($(DEBUG),)
	DEFAULT_CXXFLAGS += -g -fsanitize=address -static-libasan
	BUILD_DIR := $(BUILD_DIR)/debug
else
	DEFAULT_CXXFLAGS += -O3 -DNDEBUG 
	BUILD_DIR := $(BUILD_DIR)/release
endif





.PHONY: all
all: $(BIN)

-include $(DEPS)

$(BIN): $(OBJS)
	$(CXX) -o $@ $(OBJS) -static 

$(BUILD_DIR)/%.o: src/%.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -r $(BASE_BUILD_DIR)


