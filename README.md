# MicroTube
MicroTube is a streamlined video-on-demand platform designed for users who value simplicity and efficiency. Unlike traditional platforms, MicroTube focuses on delivering content without the clutter of comments or unnecessary features, allowing you to dive straight into the videos you love

## How to build
```bash
git clone https://github.com/oatpp/oatpp.git
cd oatpp\
mkdir build
cd build\

cmake ..
cmake --build . --target INSTALL

cd ..
cd ..

git clone https://github.com/oatpp/oatpp-swagger.git
cd oatpp-swagger
mkdir build
cd build\

cmake ..
cmake --build . --target INSTALL

cd ..
cd ..

git clone https://github.com/oatpp/oatpp-sqlite.git
cd oatpp-sqlite
mkdir build
cd build\

cmake .. -DOATPP_SQLITE_AMALGAMATION=ON
cmake --build . --target INSTALL

cd ..
cd ..

git clone https://github.com/S-E-M-CORE/MicroTube.git
cd MicroTube
cd Backend
cd Server
mkdir build
cd build\

cmake ..
cmake --build . 
```
