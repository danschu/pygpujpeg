git clone https://github.com/CESNET/GPUJPEG
cd gpujpeg
cmake -DCMAKE_BUILD_TYPE=Release -Bbuild .
cmake --build build --config Release