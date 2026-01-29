mkdir -p build
cd build 
rm moteur
cmake ..
make -j
./launch-moteur.sh