# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.18

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
CMAKE_COMMAND = /usr/local/lib/python3.8/dist-packages/cmake/data/bin/cmake

# The command to remove a file.
RM = /usr/local/lib/python3.8/dist-packages/cmake/data/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /mnt

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /mnt/build

# Include any dependencies generated for this target.
include lib/CMakeFiles/LICM.dir/depend.make

# Include the progress variables for this target.
include lib/CMakeFiles/LICM.dir/progress.make

# Include the compile flags for this target's objects.
include lib/CMakeFiles/LICM.dir/flags.make

lib/CMakeFiles/LICM.dir/LICM.cpp.o: lib/CMakeFiles/LICM.dir/flags.make
lib/CMakeFiles/LICM.dir/LICM.cpp.o: ../lib/LICM.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mnt/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object lib/CMakeFiles/LICM.dir/LICM.cpp.o"
	cd /mnt/build/lib && $(CMAKE_COMMAND) -E __run_co_compile --tidy="clang-tidy-12;-header-filter=.*;--extra-arg-before=--driver-mode=g++" --source=/mnt/lib/LICM.cpp -- /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/LICM.dir/LICM.cpp.o -c /mnt/lib/LICM.cpp

lib/CMakeFiles/LICM.dir/LICM.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/LICM.dir/LICM.cpp.i"
	cd /mnt/build/lib && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /mnt/lib/LICM.cpp > CMakeFiles/LICM.dir/LICM.cpp.i

lib/CMakeFiles/LICM.dir/LICM.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/LICM.dir/LICM.cpp.s"
	cd /mnt/build/lib && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /mnt/lib/LICM.cpp -o CMakeFiles/LICM.dir/LICM.cpp.s

lib/CMakeFiles/LICM.dir/RegAllocIntfGraph.cpp.o: lib/CMakeFiles/LICM.dir/flags.make
lib/CMakeFiles/LICM.dir/RegAllocIntfGraph.cpp.o: ../lib/RegAllocIntfGraph.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mnt/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object lib/CMakeFiles/LICM.dir/RegAllocIntfGraph.cpp.o"
	cd /mnt/build/lib && $(CMAKE_COMMAND) -E __run_co_compile --tidy="clang-tidy-12;-header-filter=.*;--extra-arg-before=--driver-mode=g++" --source=/mnt/lib/RegAllocIntfGraph.cpp -- /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/LICM.dir/RegAllocIntfGraph.cpp.o -c /mnt/lib/RegAllocIntfGraph.cpp

lib/CMakeFiles/LICM.dir/RegAllocIntfGraph.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/LICM.dir/RegAllocIntfGraph.cpp.i"
	cd /mnt/build/lib && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /mnt/lib/RegAllocIntfGraph.cpp > CMakeFiles/LICM.dir/RegAllocIntfGraph.cpp.i

lib/CMakeFiles/LICM.dir/RegAllocIntfGraph.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/LICM.dir/RegAllocIntfGraph.cpp.s"
	cd /mnt/build/lib && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /mnt/lib/RegAllocIntfGraph.cpp -o CMakeFiles/LICM.dir/RegAllocIntfGraph.cpp.s

# Object files for target LICM
LICM_OBJECTS = \
"CMakeFiles/LICM.dir/LICM.cpp.o" \
"CMakeFiles/LICM.dir/RegAllocIntfGraph.cpp.o"

# External object files for target LICM
LICM_EXTERNAL_OBJECTS =

lib/libLICM.so: lib/CMakeFiles/LICM.dir/LICM.cpp.o
lib/libLICM.so: lib/CMakeFiles/LICM.dir/RegAllocIntfGraph.cpp.o
lib/libLICM.so: lib/CMakeFiles/LICM.dir/build.make
lib/libLICM.so: lib/CMakeFiles/LICM.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/mnt/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX shared library libLICM.so"
	cd /mnt/build/lib && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/LICM.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
lib/CMakeFiles/LICM.dir/build: lib/libLICM.so

.PHONY : lib/CMakeFiles/LICM.dir/build

lib/CMakeFiles/LICM.dir/clean:
	cd /mnt/build/lib && $(CMAKE_COMMAND) -P CMakeFiles/LICM.dir/cmake_clean.cmake
.PHONY : lib/CMakeFiles/LICM.dir/clean

lib/CMakeFiles/LICM.dir/depend:
	cd /mnt/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /mnt /mnt/lib /mnt/build /mnt/build/lib /mnt/build/lib/CMakeFiles/LICM.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : lib/CMakeFiles/LICM.dir/depend

