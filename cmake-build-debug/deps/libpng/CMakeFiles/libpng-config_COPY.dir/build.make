# CMAKE generated file: DO NOT EDIT!
# Generated by "MinGW Makefiles" Generator, CMake Version 3.17

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

# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

SHELL = cmd.exe

# The CMake executable.
CMAKE_COMMAND = "C:\Program Files\JetBrains\CLion 2020.2.5\bin\cmake\win\bin\cmake.exe"

# The command to remove a file.
RM = "C:\Program Files\JetBrains\CLion 2020.2.5\bin\cmake\win\bin\cmake.exe" -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = C:\Users\FAMILY\CLionProjects\framedisplay

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = C:\Users\FAMILY\CLionProjects\framedisplay\cmake-build-debug

# Utility rule file for libpng-config_COPY.

# Include the progress variables for this target.
include deps/libpng/CMakeFiles/libpng-config_COPY.dir/progress.make

deps/libpng/CMakeFiles/libpng-config_COPY: deps/libpng/lib/libpng-config


deps/libpng/lib/libpng-config: deps/libpng/libpng16d.a
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=C:\Users\FAMILY\CLionProjects\framedisplay\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Generating lib/libpng-config, libpng-config"
	cd /d C:\Users\FAMILY\CLionProjects\framedisplay\cmake-build-debug\deps\libpng && "C:\Program Files\JetBrains\CLion 2020.2.5\bin\cmake\win\bin\cmake.exe" -E copy_if_different libpng16-config lib/libpng-config
	cd /d C:\Users\FAMILY\CLionProjects\framedisplay\cmake-build-debug\deps\libpng && "C:\Program Files\JetBrains\CLion 2020.2.5\bin\cmake\win\bin\cmake.exe" -E copy_if_different libpng16-config C:/Users/FAMILY/CLionProjects/framedisplay/cmake-build-debug/deps/libpng/libpng-config

deps/libpng/libpng-config: deps/libpng/lib/libpng-config
	@$(CMAKE_COMMAND) -E touch_nocreate deps\libpng\libpng-config

libpng-config_COPY: deps/libpng/CMakeFiles/libpng-config_COPY
libpng-config_COPY: deps/libpng/lib/libpng-config
libpng-config_COPY: deps/libpng/libpng-config
libpng-config_COPY: deps/libpng/CMakeFiles/libpng-config_COPY.dir/build.make

.PHONY : libpng-config_COPY

# Rule to build all files generated by this target.
deps/libpng/CMakeFiles/libpng-config_COPY.dir/build: libpng-config_COPY

.PHONY : deps/libpng/CMakeFiles/libpng-config_COPY.dir/build

deps/libpng/CMakeFiles/libpng-config_COPY.dir/clean:
	cd /d C:\Users\FAMILY\CLionProjects\framedisplay\cmake-build-debug\deps\libpng && $(CMAKE_COMMAND) -P CMakeFiles\libpng-config_COPY.dir\cmake_clean.cmake
.PHONY : deps/libpng/CMakeFiles/libpng-config_COPY.dir/clean

deps/libpng/CMakeFiles/libpng-config_COPY.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "MinGW Makefiles" C:\Users\FAMILY\CLionProjects\framedisplay C:\Users\FAMILY\CLionProjects\framedisplay\deps\libpng C:\Users\FAMILY\CLionProjects\framedisplay\cmake-build-debug C:\Users\FAMILY\CLionProjects\framedisplay\cmake-build-debug\deps\libpng C:\Users\FAMILY\CLionProjects\framedisplay\cmake-build-debug\deps\libpng\CMakeFiles\libpng-config_COPY.dir\DependInfo.cmake --color=$(COLOR)
.PHONY : deps/libpng/CMakeFiles/libpng-config_COPY.dir/depend

