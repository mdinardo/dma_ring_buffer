ifeq ($(OS),Windows_NT)
  ifeq ($(shell uname -s),) # not in a bash-like shell
	CLEANUP = del /F /Q
	MKDIR = mkdir
  else # in a bash-like shell, like msys
	CLEANUP = rm -f
	MKDIR = mkdir -p
  endif
	TARGET_EXTENSION=.exe
else
	CLEANUP = rm -f
	MKDIR = mkdir -p
	TARGET_EXTENSION=out
endif

.PHONY: clean_build
.PHONY: clean_test
.PHONY: clean
.PHONY: test
.PHONY: test_debug
.PHONY: build
.PHONY: build_debug

PATHU = unity/src/
PATHS = src/
PATHT = test/
PATHB = build/
PATHD = build/depends/
PATHO = build/objs/
PATHTO = build/test_objs/
PATHR = build/results/

BUILD_PATHS = $(PATHB) $(PATHD) $(PATHO) $(PATHTO) $(PATHR)

SRCS = $(wildcard $(PATHS)*.c)
SRCT = $(wildcard $(PATHT)*.c)

INCLUDE_PATHS = . $(PATHU) $(PATHS)
INCFLAGS=$(addprefix -I,$(INCLUDE_PATHS))

COMPILE=gcc -c
LINK=gcc
DEPEND=gcc -MM -MG -MF
CFLAGS = $(INCFLAGS) -Wall
LDFLAGS=

# names of test c files must begin with this prefix.
# No spaces! all whitespace is included
TEST_PREFIX:=Test_

RESULTS = $(patsubst $(PATHT)$(TEST_PREFIX)%.c,$(PATHR)$(TEST_PREFIX)%.txt,$(SRCT) )

PASSED = `grep -s PASS $(PATHR)*.txt`
FAIL = `grep -s FAIL $(PATHR)*.txt`
IGNORE = `grep -s IGNORE $(PATHR)*.txt`

build_debug: CFLAGS += -DSTREAM_RING_BUFFER_DEBUG
build_debug: build

build: BUILD_PATHS = $(PATHB) $(PATHD) $(PATHO)
build: $(BUILD_PATHS) $(patsubst $(PATHS)%.c,$(PATHO)%.o,$(SRCS) )

test_debug: CFLAGS += -DTEST
test_debug: build_debug test

test: CFLAGS += -DTEST
test: $(BUILD_PATHS) $(RESULTS)
	@echo "-----------------------\nIGNORES:\n-----------------------"
	@echo "$(IGNORE)"
	@echo "-----------------------\nFAILURES:\n-----------------------"
	@echo "$(FAIL)"
	@echo "-----------------------\nPASSED:\n-----------------------"
	@echo "$(PASSED)"
	@echo "\nDONE"

$(PATHR)%.txt: $(PATHB)%.$(TARGET_EXTENSION)
	-./$< > $@ 2>&1

$(PATHB)$(TEST_PREFIX)%.$(TARGET_EXTENSION): $(PATHTO)$(TEST_PREFIX)%.o $(PATHU)unity.o $(PATHO)%.o #$(PATHD)$(TEST_PREFIX)%.d
	$(LINK) $(LDFLAGS) -o $@ $^

# compile target code; goes into object directory
$(PATHO)%.o:: $(PATHS)%.c
	@echo Trying to build obj $(@)
	$(COMPILE) $(CFLAGS) $< -o $@

# compile unit test object files; goes into test-specific object directory
$(PATHTO)$(TEST_PREFIX)%.o: CFLAGS += -Wno-incompatible-pointer-types -Wno-pointer-sign # disable these errors to clean up terminal output;
$(PATHTO)$(TEST_PREFIX)%.o:: $(PATHT)$(TEST_PREFIX)%.c 
	@echo Trying to build test obj with prefix $(@)
	$(COMPILE) $(CFLAGS) -O0 $< -o $@

# compile unity and stick the object file with the test objects
$(PATHTO)%.o:: $(PATHU)%.c $(PATHU)%.h
	$(COMPILE) $(CFLAGS) $< -o $@
#

$(PATHD)%.d:: $(PATHT)%.c
	$(DEPEND) $@ $<

$(PATHB):
	$(MKDIR) $(PATHB)

$(PATHD):
	$(MKDIR) $(PATHD)

$(PATHTO):
	$(MKDIR) $(PATHTO)

$(PATHO):
	$(MKDIR) $(PATHO)

$(PATHR):
	$(MKDIR) $(PATHR)

clean_build:
	$(CLEANUP) $(PATHO)*.o

clean_test:
	$(CLEANUP) $(PATHTO)*.o
	$(CLEANUP) $(PATHB)*.$(TARGET_EXTENSION)
	$(CLEANUP) $(PATHR)*.txt

clean: clean_build clean_test

.PRECIOUS: $(PATHB)$(TEST_PREFIX)%.$(TARGET_EXTENSION)
.PRECIOUS: $(PATHD)%.d
.PRECIOUS: $(PATHO)%.o
.PRECIOUS: $(PATHTO)%.o
.PRECIOUS: $(PATHR)%.txt
