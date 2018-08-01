
# Dependencies : libczmq-dev malamute Qt5(Widgets, Charts)

## Directories for building dependencies.
> mkdir build_root
> export DEPENDS_ROOT=$PWD/build_root
> cd ./external


## ZeroMQ, CZMQ, Malamute, FTY-proto

Lets get ZeroMQ + Malamute into dependencies.
Please, install packages "libtool m4 automake"

```
git clone git://github.com/jedisct1/libsodium.git
git clone git://github.com/zeromq/libzmq.git
git clone git://github.com/zeromq/czmq.git
git clone git://github.com/zeromq/malamute.git
git clone git://github.com/42ity/fty-proto.git
for project in libsodium libzmq czmq malamute fty-proto; do
    cd $project
    ./autogen.sh
    CFLAGS="-I$DEPENDS_ROOT/include" LDFLAGS="-L$DEPENDS_ROOT/lib" ./configure --prefix=$DEPENDS_ROOT 
    make install
done
```

## Qt5
Install it somewhere and pass these variables to cmake:
> export QT5_SEARCH_PATH=~/Qt/5.10
> export QT_QMAKE_EXECUTABLE=$QT5_SEARCH_PATH/gcc_64/bin/qmake

# Configure/Build the project using CMake

```
mkdir build && cd build
cmake .. -DQT5_SEARCH_PATH=$QT5_SEARCH_PATH -DQT_QMAKE_EXECUTABLE=$QT_QMAKE_EXECUTABLE -DMEMADDR_SANITIZER=0 -DBUILD_UI=1
make -j$(nproc)
```
You can avoid compiling UI part by disabling a cmake option: -DBUILD_UI=0 .

# Launch the software
> export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$DEPENDS_ROOT/lib

## Run the "malamute" message broker with tau1 name
> malamute tau1


