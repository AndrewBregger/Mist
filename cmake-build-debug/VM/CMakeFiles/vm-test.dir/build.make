# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.13

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /Applications/CLion.app/Contents/bin/cmake/mac/bin/cmake

# The command to remove a file.
RM = /Applications/CLion.app/Contents/bin/cmake/mac/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/andy/Documents/Mist

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/andy/Documents/Mist/cmake-build-debug

# Include any dependencies generated for this target.
include VM/CMakeFiles/vm-test.dir/depend.make

# Include the progress variables for this target.
include VM/CMakeFiles/vm-test.dir/progress.make

# Include the compile flags for this target's objects.
include VM/CMakeFiles/vm-test.dir/flags.make

VM/CMakeFiles/vm-test.dir/main.cpp.o: VM/CMakeFiles/vm-test.dir/flags.make
VM/CMakeFiles/vm-test.dir/main.cpp.o: ../VM/main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/andy/Documents/Mist/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object VM/CMakeFiles/vm-test.dir/main.cpp.o"
	cd /Users/andy/Documents/Mist/cmake-build-debug/VM && /usr/local/Cellar/gcc@7/7.3.0/bin/g++-7  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/vm-test.dir/main.cpp.o -c /Users/andy/Documents/Mist/VM/main.cpp

VM/CMakeFiles/vm-test.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/vm-test.dir/main.cpp.i"
	cd /Users/andy/Documents/Mist/cmake-build-debug/VM && /usr/local/Cellar/gcc@7/7.3.0/bin/g++-7 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/andy/Documents/Mist/VM/main.cpp > CMakeFiles/vm-test.dir/main.cpp.i

VM/CMakeFiles/vm-test.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/vm-test.dir/main.cpp.s"
	cd /Users/andy/Documents/Mist/cmake-build-debug/VM && /usr/local/Cellar/gcc@7/7.3.0/bin/g++-7 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/andy/Documents/Mist/VM/main.cpp -o CMakeFiles/vm-test.dir/main.cpp.s

# Object files for target vm-test
vm__test_OBJECTS = \
"CMakeFiles/vm-test.dir/main.cpp.o"

# External object files for target vm-test
vm__test_EXTERNAL_OBJECTS =

VM/vm-test: VM/CMakeFiles/vm-test.dir/main.cpp.o
VM/vm-test: VM/CMakeFiles/vm-test.dir/build.make
VM/vm-test: ../VM/lib/libvmlib.a
VM/vm-test: VM/CMakeFiles/vm-test.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/andy/Documents/Mist/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable vm-test"
	cd /Users/andy/Documents/Mist/cmake-build-debug/VM && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/vm-test.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
VM/CMakeFiles/vm-test.dir/build: VM/vm-test

.PHONY : VM/CMakeFiles/vm-test.dir/build

VM/CMakeFiles/vm-test.dir/clean:
	cd /Users/andy/Documents/Mist/cmake-build-debug/VM && $(CMAKE_COMMAND) -P CMakeFiles/vm-test.dir/cmake_clean.cmake
.PHONY : VM/CMakeFiles/vm-test.dir/clean

VM/CMakeFiles/vm-test.dir/depend:
	cd /Users/andy/Documents/Mist/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/andy/Documents/Mist /Users/andy/Documents/Mist/VM /Users/andy/Documents/Mist/cmake-build-debug /Users/andy/Documents/Mist/cmake-build-debug/VM /Users/andy/Documents/Mist/cmake-build-debug/VM/CMakeFiles/vm-test.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : VM/CMakeFiles/vm-test.dir/depend
