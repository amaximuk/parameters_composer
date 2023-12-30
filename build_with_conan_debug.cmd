mkdir build_conan
cd build_conan
conan install .. --build=missing --settings=build_type=Debug
cmake -DPACKAGE_MANAGER=CONAN -G "Visual Studio 17 2022" -A x64 ..
cd ..