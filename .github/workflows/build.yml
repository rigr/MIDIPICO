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
    - name: Install dependencies
      run: |
        sudo apt update
        sudo apt install -y cmake gcc-arm-none-eabi libnewlib-arm-none-eabi libusb-1.0-0-dev pkg-config
    - name: Build
      run: |
        mkdir build
        cd build
        cmake ..
        make
    - name: Upload artifact
      uses: actions/upload-artifact@v4
      with:
        name: midipico-uf2
        path: build/midipico.uf2
