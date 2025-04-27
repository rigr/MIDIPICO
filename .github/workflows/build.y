name: Build MIDIPICO
on:
  push:
    branches: [ main ]
  pull_request:
    branches: [ main ]
jobs:
  build:
    runs-on: ubuntu-latest
    steps:
    - uses: actions/checkout@v4
      with:
        submodules: recursive
        fetch-depth: 0
    - name: Update Submodules
      run: |
        git submodule sync --recursive
        git submodule update --init --recursive --force
        cd lib/pico-sdk && git checkout master && cd ../..
        cd lib/ring_buffer_lib && git checkout main && cd ../..
        cd lib/tinyusb && git checkout master && cd ../..
    - name: Debug Submodules and Local Copy
      run: |
        ls -la lib/
        ls -la lib/local_pio_midi_uart
        cat lib/local_pio_midi_uart/CMakeLists.txt
        cat CMakeLists.txt
        find . -name pio_midi_uart.pio.h
    - name: Install dependencies
      run: |
        sudo apt update
        sudo apt install -y cmake gcc-arm-none-eabi libnewlib-arm-none-eabi libusb-1.0-0-dev pkg-config
    - name: Build
      env:
        PICO_SDK_PATH: ${{ github.workspace }}/lib/pico-sdk
      run: |
        mkdir build
        cd build
        rm -rf CMakeCache.txt CMakeFiles
        cmake .. -DCMAKE_VERBOSE_MAKEFILE=ON
        make
        find . -name pio_midi_uart.pio.h
    - name: Upload artifact
      uses: actions/upload-artifact@v4
      with:
        name: midipico-uf2
        path: build/midipico.uf2
