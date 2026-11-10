CXX        := g++
BUILD_MODE := $(strip debug)
TRACE      := $(strip 0)
SRC_DIR    := srcs
TEST_DIR   := tests
DEBUG_DIR  := debug
INC_DIR    := headers

# TRACE=1 dung thu muc build rieng de khong lan file .o khac co DLOG
BUILD_DIR  := $(strip build/$(BUILD_MODE)$(if $(filter 1,$(TRACE)),-trace))

ifeq ($(OS),Windows_NT)
    EXE := .exe
endif
APP_BIN   := $(BUILD_DIR)/notes$(EXE)
TEST_BIN  := $(BUILD_DIR)/notes_test$(EXE)
DEBUG_BIN := $(BUILD_DIR)/notes_debug$(EXE)

CXXFLAGS := -std=c++17 -Wall -Wextra -Werror=return-type -I$(INC_DIR) -I$(TEST_DIR)
ifeq ($(BUILD_MODE),release)
    CXXFLAGS += -O2 -DNDEBUG
else ifeq ($(BUILD_MODE),check)
    CXXFLAGS += -g -O0 -D_GLIBCXX_DEBUG -D_GLIBCXX_ASSERTIONS
else
    CXXFLAGS += -g -O0
endif
ifeq ($(TRACE),1)
    CXXFLAGS += -DNOTES_DEBUG
endif
DEPFLAGS := -MMD -MP

rwildcard = $(foreach d,$(wildcard $(1)/*),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

LIB_SRCS   := $(filter-out $(SRC_DIR)/main.cpp $(SRC_DIR)/testing%.cpp,$(call rwildcard,$(SRC_DIR),*.cpp))
LIB_OBJS   := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(LIB_SRCS))
APP_OBJ    := $(BUILD_DIR)/main.o
TEST_SRCS  := $(call rwildcard,$(TEST_DIR),*.cpp)
TEST_OBJS  := $(patsubst $(TEST_DIR)/%.cpp,$(BUILD_DIR)/tests/%.o,$(TEST_SRCS))
DEBUG_SRCS := $(call rwildcard,$(DEBUG_DIR),*.cpp)
DEBUG_OBJS := $(patsubst $(DEBUG_DIR)/%.cpp,$(BUILD_DIR)/dbgtool/%.o,$(DEBUG_SRCS))
ALL_OBJS   := $(LIB_OBJS) $(APP_OBJ) $(TEST_OBJS) $(DEBUG_OBJS)

.PHONY: all build-app build-test run run-trace test test-trace test-check \
        debug-run release clean clean-test rebuild print-vars

all: $(APP_BIN)
build-app: $(APP_BIN)
build-test: $(TEST_BIN)

run: $(APP_BIN)
	./$(APP_BIN)

run-trace:
	$(MAKE) TRACE=1 run

test: $(TEST_BIN)
	./$(TEST_BIN)

test-trace:
	$(MAKE) TRACE=1 test

test-check:
	$(MAKE) BUILD_MODE=check test

debug-run: $(DEBUG_BIN)
	./$(DEBUG_BIN) --step

release:
	$(MAKE) BUILD_MODE=release

clean:
	rm -rf build

clean-test:
	rm -f vault/notes/zz_test*.md vault/notes/zz_dbg_*.md

rebuild: clean all

print-vars:
	@echo "BUILD_DIR=[$(BUILD_DIR)]"
	@echo "LIB_SRCS=[$(LIB_SRCS)]"
	@echo "TEST_SRCS=[$(TEST_SRCS)]"
	@echo "DEBUG_SRCS=[$(DEBUG_SRCS)]"

$(APP_BIN): $(APP_OBJ) $(LIB_OBJS)
	$(CXX) $^ -o $@

$(TEST_BIN): $(TEST_OBJS) $(LIB_OBJS)
	$(CXX) $^ -o $@

$(DEBUG_BIN): $(DEBUG_OBJS) $(LIB_OBJS)
	$(CXX) $^ -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

$(BUILD_DIR)/tests/%.o: $(TEST_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

$(BUILD_DIR)/dbgtool/%.o: $(DEBUG_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

-include $(ALL_OBJS:.o=.d)