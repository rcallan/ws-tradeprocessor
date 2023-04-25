Here are a few steps for building this project on linux. Please navigate to the top level directory for this repo and execute the following commands

```
mkdir build && cd build
cmake .. -B .
sudo cmake --build . --target install
```

A `tradeprocessor` executable should now be located in the `bin` directory