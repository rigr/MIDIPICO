name: Build MIDIPICO UF2

on:
  push:
    branches: [ main ]
  pull_request:
    branches: [ main ]

jobs:
  build:
    runs-on: ubuntu-latest

    steps:
    - name: Checkout repository
      uses: actions/checkout@v4

    - name: Install ARM GCC Compiler
      run: |
        sudo apt-get update
        sudo apt-get install -y gcc-arm-none-eabi

    - name: Clone pico-sdk
      run: |
        git clone --depth 1 https://github.com/raspberrypi/pico-sdk.git
        cd pico-sdk
        git submodule update --init

    - name: Clone usb_midi_host
      run: |
        git clone --depth 1 https://github.com/rppicomidi/usb_midi_host.git

    - name: Clone pio_midi_uart_lib
      run: |
        git clone --depth 1 https://github.com/rppicomidi/pio_midi_uart_lib.git

    - name: Clone Pico-PIO-USB
      run: |
        git clone --depth 1 https://github.com/sekigon-gonnoc/Pico-PIO-USB.git
        mv Pico-PIO-USB pico-sdk/lib/pico_pio_usb

    - name: Build UF2
      run: |
        export PICO_SDK_PATH=$GITHUB_WORKSPACE/pico-sdk
        mkdir build
        cd build
        cmake .. -DPICO_SDK_PATH=$PICO_SDK_PATH
        make

    - name: Upload UF2 Artifact
      uses: actions/upload-artifact@v4
      with:
        name: midipico.uf2
        path: build/midipico.uf2
