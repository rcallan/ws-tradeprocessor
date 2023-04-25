#### Purpose of project

This is a C++ websocket client for Finnhub trade updates which also allows you to process these updates in real time using predefined or custom calculators

#### Building the project

When cloning this repo, please use the following commands to install dependencies

```
git clone --recurse-submodules https://github.com/rcallan/ws-tradeprocessor
cd vcpkg
./bootstrap-vcpkg.sh
./vcpkg install
```

Here are a few steps for building this project on linux. Please navigate to the top level directory for this repo and execute the following commands

```
mkdir build && cd build
cmake ..
cmake --build .
```

An `rtdprocessor` executable should now be located in the `build` directory. Be sure to specify the config file you would like to use when calling the executable