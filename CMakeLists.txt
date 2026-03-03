# Set minimum required version of CMake
cmake_minimum_required(VERSION 3.12)

# Set board type because we are building for PicoW
set(PICO_BOARD pico_w)

# Include build functions from Pico SDK
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

# Set name of project (as PROJECT_NAME) and C/C   standards
project(Garage_Door C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Creates a pico-sdk subdirectory in our project for the libraries
pico_sdk_init()

add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        -Wno-maybe-uninitialized
)

# Tell CMake where to find the executable source file
add_executable(${PROJECT_NAME}
        main.cpp
        Hardware/GPIOPin.cpp
        Hardware/GPIOPin.h
        Hardware/RotaryEncoder.cpp
        Hardware/RotaryEncoder.h
        Hardware/StepperMotor.cpp
        Hardware/StepperMotor.h
        Hardware/LimitSwitch.cpp
        Hardware/LimitSwitch.h
        Application/GarageDoor.cpp
        Application/GarageDoor.h
        Hardware/SwitchPair.cpp
        Hardware/SwitchPair.h
        Application/StateMachine.cpp
        Application/StateMachine.h
        Hardware/Leds.cpp
        Hardware/Leds.h
        Application/MQTTService.cpp
        Application/MQTTService.h
        Hardware/OLEDDisplay.cpp
        Hardware/OLEDDisplay.h
        pico-modbus/display/framebuf.cpp
        pico-modbus/display/framebuf.h
        pico-modbus/display/mono_vlsb.cpp
        pico-modbus/display/mono_vlsb.h
        pico-modbus/display/ssd1306.cpp
        pico-modbus/display/ssd1306.h
        pico-modbus/display/i2c_device.h
        pico-modbus/i2c/PicoI2CBus.cpp
        pico-modbus/i2c/PicoI2CBus.h
        pico-modbus/i2c/PicoI2CDevice.cpp
        pico-modbus/i2c/PicoI2CDevice.h
        private.h
        pico-modbus/mqtt/IPStack.cpp
        pico-modbus/mqtt/Countdown.cpp
        pico-modbus/paho.mqtt.embedded-c/MQTTPacket/src/MQTTConnectClient.c
        pico-modbus/paho.mqtt.embedded-c/MQTTPacket/src/MQTTDeserializePublish.c
        pico-modbus/paho.mqtt.embedded-c/MQTTPacket/src/MQTTFormat.c
        pico-modbus/paho.mqtt.embedded-c/MQTTPacket/src/MQTTPacket.c
        pico-modbus/paho.mqtt.embedded-c/MQTTPacket/src/MQTTSerializePublish.c
        pico-modbus/paho.mqtt.embedded-c/MQTTPacket/src/MQTTSubscribeClient.c
        pico-modbus/paho.mqtt.embedded-c/MQTTPacket/src/MQTTUnsubscribeClient.c
)

# Create map/bin/hex/uf2 files
pico_add_extra_outputs(${PROJECT_NAME})

# Link to pico_stdlib (gpio, time, etc. functions)
target_link_libraries(${PROJECT_NAME}
        pico_stdlib
        hardware_i2c
        hardware_spi
        hardware_gpio
        pico_cyw43_arch_lwip_poll


)

target_include_directories(${PROJECT_NAME} PRIVATE
        pico-modbus/mqtt
        pico-modbus/paho.mqtt.embedded-c/MQTTPacket/src
        pico-modbus/paho.mqtt.embedded-c/MQTTClient/src
        pico-modbus
        pico-modbus/display/fonts
        pico-modbus/display
        pico-modbus/i2c

        ${CMAKE_CURRENT_LIST_DIR}/include
)



# Disable usb output, enable uart output
pico_enable_stdio_usb(${PROJECT_NAME} 0)
pico_enable_stdio_uart(${PROJECT_NAME} 1)