mkdir -p build
cd build 
rm src
cmake ..
make -j
./launch-src.sh