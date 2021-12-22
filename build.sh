#!/bin/bash
make -j8 install
cp ~/Vitens/projecten/packages/viphreeqc/build/lib/libiphreeqc.dylib ~/Vitens/projecten/packages/phreeqpython/phreeqpython/lib/viphreeqc.dylib
python3 -m nose -v ~/Vitens/projecten/packages/phreeqpython
