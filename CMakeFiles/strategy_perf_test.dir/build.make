# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 4.0

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

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
CMAKE_COMMAND = /opt/homebrew/bin/cmake

# The command to remove a file.
RM = /opt/homebrew/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/jadenfix/financial_cpp_copy

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/jadenfix/financial_cpp_copy

# Include any dependencies generated for this target.
include CMakeFiles/strategy_perf_test.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/strategy_perf_test.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/strategy_perf_test.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/strategy_perf_test.dir/flags.make

CMakeFiles/strategy_perf_test.dir/codegen:
.PHONY : CMakeFiles/strategy_perf_test.dir/codegen

CMakeFiles/strategy_perf_test.dir/strategy_performance_test.cpp.o: CMakeFiles/strategy_perf_test.dir/flags.make
CMakeFiles/strategy_perf_test.dir/strategy_performance_test.cpp.o: strategy_performance_test.cpp
CMakeFiles/strategy_perf_test.dir/strategy_performance_test.cpp.o: CMakeFiles/strategy_perf_test.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/Users/jadenfix/financial_cpp_copy/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/strategy_perf_test.dir/strategy_performance_test.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/strategy_perf_test.dir/strategy_performance_test.cpp.o -MF CMakeFiles/strategy_perf_test.dir/strategy_performance_test.cpp.o.d -o CMakeFiles/strategy_perf_test.dir/strategy_performance_test.cpp.o -c /Users/jadenfix/financial_cpp_copy/strategy_performance_test.cpp

CMakeFiles/strategy_perf_test.dir/strategy_performance_test.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/strategy_perf_test.dir/strategy_performance_test.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/jadenfix/financial_cpp_copy/strategy_performance_test.cpp > CMakeFiles/strategy_perf_test.dir/strategy_performance_test.cpp.i

CMakeFiles/strategy_perf_test.dir/strategy_performance_test.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/strategy_perf_test.dir/strategy_performance_test.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/jadenfix/financial_cpp_copy/strategy_performance_test.cpp -o CMakeFiles/strategy_perf_test.dir/strategy_performance_test.cpp.s

# Object files for target strategy_perf_test
strategy_perf_test_OBJECTS = \
"CMakeFiles/strategy_perf_test.dir/strategy_performance_test.cpp.o"

# External object files for target strategy_perf_test
strategy_perf_test_EXTERNAL_OBJECTS =

strategy_perf_test: CMakeFiles/strategy_perf_test.dir/strategy_performance_test.cpp.o
strategy_perf_test: CMakeFiles/strategy_perf_test.dir/build.make
strategy_perf_test: libtrading_system_lib.a
strategy_perf_test: CMakeFiles/strategy_perf_test.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/Users/jadenfix/financial_cpp_copy/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable strategy_perf_test"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/strategy_perf_test.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/strategy_perf_test.dir/build: strategy_perf_test
.PHONY : CMakeFiles/strategy_perf_test.dir/build

CMakeFiles/strategy_perf_test.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/strategy_perf_test.dir/cmake_clean.cmake
.PHONY : CMakeFiles/strategy_perf_test.dir/clean

CMakeFiles/strategy_perf_test.dir/depend:
	cd /Users/jadenfix/financial_cpp_copy && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/jadenfix/financial_cpp_copy /Users/jadenfix/financial_cpp_copy /Users/jadenfix/financial_cpp_copy /Users/jadenfix/financial_cpp_copy /Users/jadenfix/financial_cpp_copy/CMakeFiles/strategy_perf_test.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : CMakeFiles/strategy_perf_test.dir/depend

