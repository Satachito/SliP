#	  . ~/esp/esp-idf/export.sh
#	  cd Tab5 && idf.py build flash monitor
#
#	Needs the RISC-V toolchain, which install.sh does not fetch unless a RISC-V
#	target is asked for:
#
#	  ~/esp/esp-idf/install.sh esp32p4

cmake_minimum_required( VERSION 3.16 )
include( $ENV{IDF_PATH}/tools/cmake/project.cmake )
project( slip )
