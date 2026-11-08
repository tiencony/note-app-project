CXX        := g++
BUILD_MODE := $(strip debug)
SRC_DIR    := srcs
INC_DIR    := headers
BUILD_DIR  := $(strip build/$(BUILD_MODE))

ifeq ($(OS),Windows_NT)
    EXE := .exe
endif
TARGET := $(BUILD_DIR)/notes$(EXE)

CXXFLAGS := -std=c++17 -Wall -Wextra -I$(INC_DIR)
ifeq ($(BUILD_MODE),release)
    CXXFLAGS += -O2
else
    CXXFLAGS += -g -O0
endif

DEPFLAGS := -MMD -MP

SRCS := $(filter-out $(SRC_DIR)/testing%.cpp,$(wildcard $(SRC_DIR)/*.cpp))
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))

.PHONY: all run test release clean clean-test rebuild print-vars

all: $(TARGET)

run: $(TARGET)
	./$(TARGET)

test: $(TARGET)
	./$(TARGET) --test

release:
	$(MAKE) BUILD_MODE=release

clean:
	rm -rf build

clean-test:
	rm -f vault/notes/zz_test_*.md

rebuild: clean all

print-vars:
	@echo "BUILD_DIR=[$(BUILD_DIR)]"
	@echo "TARGET=[$(TARGET)]"
	@echo "SRCS=[$(SRCS)]"
	@echo "OBJS=[$(OBJS)]"

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $@

-include $(OBJS:.o=.d)