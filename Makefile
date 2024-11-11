# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.30

# Default target executed when no arguments are given to make.
default_target: all
.PHONY : default_target

# Allow only one "make -f Makefile2" at a time, but pass parallelism.
.NOTPARALLEL:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /opt/homebrew/Cellar/cmake/3.30.5/bin/cmake

# The command to remove a file.
RM = /opt/homebrew/Cellar/cmake/3.30.5/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/yangjeep/ws/yangjeep/playground_c_search

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/yangjeep/ws/yangjeep/playground_c_search

#=============================================================================
# Targets provided globally by CMake.

# Special rule for the target test
test:
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --cyan "Running tests..."
	/opt/homebrew/Cellar/cmake/3.30.5/bin/ctest --force-new-ctest-process $(ARGS)
.PHONY : test

# Special rule for the target test
test/fast: test
.PHONY : test/fast

# Special rule for the target edit_cache
edit_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --cyan "Running CMake cache editor..."
	/opt/homebrew/Cellar/cmake/3.30.5/bin/ccmake -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)
.PHONY : edit_cache

# Special rule for the target edit_cache
edit_cache/fast: edit_cache
.PHONY : edit_cache/fast

# Special rule for the target rebuild_cache
rebuild_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --cyan "Running CMake to regenerate build system..."
	/opt/homebrew/Cellar/cmake/3.30.5/bin/cmake --regenerate-during-build -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)
.PHONY : rebuild_cache

# Special rule for the target rebuild_cache
rebuild_cache/fast: rebuild_cache
.PHONY : rebuild_cache/fast

# Special rule for the target list_install_components
list_install_components:
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --cyan "Available install components are: \"Unspecified\" \"library\""
.PHONY : list_install_components

# Special rule for the target list_install_components
list_install_components/fast: list_install_components
.PHONY : list_install_components/fast

# Special rule for the target install
install: preinstall
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --cyan "Install the project..."
	/opt/homebrew/Cellar/cmake/3.30.5/bin/cmake -P cmake_install.cmake
.PHONY : install

# Special rule for the target install
install/fast: preinstall/fast
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --cyan "Install the project..."
	/opt/homebrew/Cellar/cmake/3.30.5/bin/cmake -P cmake_install.cmake
.PHONY : install/fast

# Special rule for the target install/local
install/local: preinstall
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --cyan "Installing only the local directory..."
	/opt/homebrew/Cellar/cmake/3.30.5/bin/cmake -DCMAKE_INSTALL_LOCAL_ONLY=1 -P cmake_install.cmake
.PHONY : install/local

# Special rule for the target install/local
install/local/fast: preinstall/fast
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --cyan "Installing only the local directory..."
	/opt/homebrew/Cellar/cmake/3.30.5/bin/cmake -DCMAKE_INSTALL_LOCAL_ONLY=1 -P cmake_install.cmake
.PHONY : install/local/fast

# Special rule for the target install/strip
install/strip: preinstall
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --cyan "Installing the project stripped..."
	/opt/homebrew/Cellar/cmake/3.30.5/bin/cmake -DCMAKE_INSTALL_DO_STRIP=1 -P cmake_install.cmake
.PHONY : install/strip

# Special rule for the target install/strip
install/strip/fast: preinstall/fast
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --cyan "Installing the project stripped..."
	/opt/homebrew/Cellar/cmake/3.30.5/bin/cmake -DCMAKE_INSTALL_DO_STRIP=1 -P cmake_install.cmake
.PHONY : install/strip/fast

# The main all target
all: cmake_check_build_system
	$(CMAKE_COMMAND) -E cmake_progress_start /Users/yangjeep/ws/yangjeep/playground_c_search/CMakeFiles /Users/yangjeep/ws/yangjeep/playground_c_search//CMakeFiles/progress.marks
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 all
	$(CMAKE_COMMAND) -E cmake_progress_start /Users/yangjeep/ws/yangjeep/playground_c_search/CMakeFiles 0
.PHONY : all

# The main clean target
clean:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 clean
.PHONY : clean

# The main clean target
clean/fast: clean
.PHONY : clean/fast

# Prepare targets for installation.
preinstall: all
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall

# Prepare targets for installation.
preinstall/fast:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall/fast

# clear depends
depend:
	$(CMAKE_COMMAND) -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 1
.PHONY : depend

#=============================================================================
# Target rules for targets named common

# Build rule for target.
common: cmake_check_build_system
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 common
.PHONY : common

# fast build rule for target.
common/fast:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/common.dir/build.make CMakeFiles/common.dir/build
.PHONY : common/fast

#=============================================================================
# Target rules for targets named test_document

# Build rule for target.
test_document: cmake_check_build_system
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 test_document
.PHONY : test_document

# fast build rule for target.
test_document/fast:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/test_document.dir/build.make CMakeFiles/test_document.dir/build
.PHONY : test_document/fast

#=============================================================================
# Target rules for targets named test_indexer

# Build rule for target.
test_indexer: cmake_check_build_system
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 test_indexer
.PHONY : test_indexer

# fast build rule for target.
test_indexer/fast:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/test_indexer.dir/build.make CMakeFiles/test_indexer.dir/build
.PHONY : test_indexer/fast

#=============================================================================
# Target rules for targets named test_server

# Build rule for target.
test_server: cmake_check_build_system
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 test_server
.PHONY : test_server

# fast build rule for target.
test_server/fast:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/test_server.dir/build.make CMakeFiles/test_server.dir/build
.PHONY : test_server/fast

#=============================================================================
# Target rules for targets named unity

# Build rule for target.
unity: cmake_check_build_system
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 unity
.PHONY : unity

# fast build rule for target.
unity/fast:
	$(MAKE) $(MAKESILENT) -f _deps/unity-build/CMakeFiles/unity.dir/build.make _deps/unity-build/CMakeFiles/unity.dir/build
.PHONY : unity/fast

src/common/document.o: src/common/document.c.o
.PHONY : src/common/document.o

# target to build an object file
src/common/document.c.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/common.dir/build.make CMakeFiles/common.dir/src/common/document.c.o
.PHONY : src/common/document.c.o

src/common/document.i: src/common/document.c.i
.PHONY : src/common/document.i

# target to preprocess a source file
src/common/document.c.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/common.dir/build.make CMakeFiles/common.dir/src/common/document.c.i
.PHONY : src/common/document.c.i

src/common/document.s: src/common/document.c.s
.PHONY : src/common/document.s

# target to generate assembly for a file
src/common/document.c.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/common.dir/build.make CMakeFiles/common.dir/src/common/document.c.s
.PHONY : src/common/document.c.s

src/common/indexer.o: src/common/indexer.c.o
.PHONY : src/common/indexer.o

# target to build an object file
src/common/indexer.c.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/common.dir/build.make CMakeFiles/common.dir/src/common/indexer.c.o
.PHONY : src/common/indexer.c.o

src/common/indexer.i: src/common/indexer.c.i
.PHONY : src/common/indexer.i

# target to preprocess a source file
src/common/indexer.c.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/common.dir/build.make CMakeFiles/common.dir/src/common/indexer.c.i
.PHONY : src/common/indexer.c.i

src/common/indexer.s: src/common/indexer.c.s
.PHONY : src/common/indexer.s

# target to generate assembly for a file
src/common/indexer.c.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/common.dir/build.make CMakeFiles/common.dir/src/common/indexer.c.s
.PHONY : src/common/indexer.c.s

test/test_document.o: test/test_document.c.o
.PHONY : test/test_document.o

# target to build an object file
test/test_document.c.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/test_document.dir/build.make CMakeFiles/test_document.dir/test/test_document.c.o
.PHONY : test/test_document.c.o

test/test_document.i: test/test_document.c.i
.PHONY : test/test_document.i

# target to preprocess a source file
test/test_document.c.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/test_document.dir/build.make CMakeFiles/test_document.dir/test/test_document.c.i
.PHONY : test/test_document.c.i

test/test_document.s: test/test_document.c.s
.PHONY : test/test_document.s

# target to generate assembly for a file
test/test_document.c.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/test_document.dir/build.make CMakeFiles/test_document.dir/test/test_document.c.s
.PHONY : test/test_document.c.s

test/test_indexer.o: test/test_indexer.c.o
.PHONY : test/test_indexer.o

# target to build an object file
test/test_indexer.c.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/test_indexer.dir/build.make CMakeFiles/test_indexer.dir/test/test_indexer.c.o
.PHONY : test/test_indexer.c.o

test/test_indexer.i: test/test_indexer.c.i
.PHONY : test/test_indexer.i

# target to preprocess a source file
test/test_indexer.c.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/test_indexer.dir/build.make CMakeFiles/test_indexer.dir/test/test_indexer.c.i
.PHONY : test/test_indexer.c.i

test/test_indexer.s: test/test_indexer.c.s
.PHONY : test/test_indexer.s

# target to generate assembly for a file
test/test_indexer.c.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/test_indexer.dir/build.make CMakeFiles/test_indexer.dir/test/test_indexer.c.s
.PHONY : test/test_indexer.c.s

test/test_server.o: test/test_server.c.o
.PHONY : test/test_server.o

# target to build an object file
test/test_server.c.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/test_server.dir/build.make CMakeFiles/test_server.dir/test/test_server.c.o
.PHONY : test/test_server.c.o

test/test_server.i: test/test_server.c.i
.PHONY : test/test_server.i

# target to preprocess a source file
test/test_server.c.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/test_server.dir/build.make CMakeFiles/test_server.dir/test/test_server.c.i
.PHONY : test/test_server.c.i

test/test_server.s: test/test_server.c.s
.PHONY : test/test_server.s

# target to generate assembly for a file
test/test_server.c.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/test_server.dir/build.make CMakeFiles/test_server.dir/test/test_server.c.s
.PHONY : test/test_server.c.s

# Help Target
help:
	@echo "The following are some of the valid targets for this Makefile:"
	@echo "... all (the default if no target is provided)"
	@echo "... clean"
	@echo "... depend"
	@echo "... edit_cache"
	@echo "... install"
	@echo "... install/local"
	@echo "... install/strip"
	@echo "... list_install_components"
	@echo "... rebuild_cache"
	@echo "... test"
	@echo "... common"
	@echo "... test_document"
	@echo "... test_indexer"
	@echo "... test_server"
	@echo "... unity"
	@echo "... src/common/document.o"
	@echo "... src/common/document.i"
	@echo "... src/common/document.s"
	@echo "... src/common/indexer.o"
	@echo "... src/common/indexer.i"
	@echo "... src/common/indexer.s"
	@echo "... test/test_document.o"
	@echo "... test/test_document.i"
	@echo "... test/test_document.s"
	@echo "... test/test_indexer.o"
	@echo "... test/test_indexer.i"
	@echo "... test/test_indexer.s"
	@echo "... test/test_server.o"
	@echo "... test/test_server.i"
	@echo "... test/test_server.s"
.PHONY : help



#=============================================================================
# Special targets to cleanup operation of make.

# Special rule to run CMake to check the build system integrity.
# No rule that depends on this can have commands that come from listfiles
# because they might be regenerated.
cmake_check_build_system:
	$(CMAKE_COMMAND) -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 0
.PHONY : cmake_check_build_system

