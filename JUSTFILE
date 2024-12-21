export WD := `pwd` / "chip_simulator_2023"
export SYSTEMC_HOME := WD / "lib" / "systemc-2.3.3"
export LD_LIBRARY_PATH := SYSTEMC_HOME / "lib-linux64"

build:
  g++ -I. -I$SYSTEMC_HOME/include -L. -L$LD_LIBRARY_PATH -lsystemc -lm -o ./mnist \
    chip_simulator_2023/test/single_core_test_mnist.cpp \
    chip_simulator_2023/core/*.cpp \
    chip_simulator_2023/prims/*.cpp \
    chip_simulator_2023/trace_engine/*.cpp \

test: build
  ./mnist