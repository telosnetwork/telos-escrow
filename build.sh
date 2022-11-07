#/bin/bash

cdt-cpp -I="./include/" -o="./escrow.wasm" -contract="escrow" -abigen ./src/escrow.cpp
