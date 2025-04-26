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
        git submodule sync
        git submodule update --init --recursive --force
        git submodule foreach git checkout main
        git submodule foreach git pull origin main
    - name: Debug Submodules
      run: |
        ls -la lib/
        ls -la lib/pico-sdk || echo "pico-sdk directory missing or empty"
        ls -la lib/tinyusb || echo "tinyusb directory missing or empty"
        ls -la lib/pio_midi_uart_lib || echo "pio_midi_uart_lib directory missing or empty"
        find lib/pio_midi_uart_lib -type f
        git submodule status
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
        cmake ..
        make
    - name: Upload artifact
      uses: actions/upload-artifact@v4
      with:
        name: midipico-uf2
        path: build/midipico.uf2
