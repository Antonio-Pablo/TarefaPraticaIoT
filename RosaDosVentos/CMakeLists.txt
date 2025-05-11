cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()

set(PICO_BOARD pico_w CACHE STRING "Board type")

# Importa o SDK do Pico
include(pico_sdk_import.cmake)

project(RosaDosVentos C CXX ASM)

# Inicializa o SDK do Pico
pico_sdk_init()

# Adiciona o executável principal
add_executable(RosaDosVentos 
    RosaDosVentos.c
)

# Nome e versão
pico_set_program_name(RosaDosVentos "RosaDosVentos")
pico_set_program_version(RosaDosVentos "0.1")

# Desativa a saída padrão via UART e USB, se necessário
pico_enable_stdio_uart(RosaDosVentos 0)
pico_enable_stdio_usb(RosaDosVentos 0)

# Adiciona diretórios de include
target_include_directories(RosaDosVentos PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

# Vincula bibliotecas necessárias
target_link_libraries(RosaDosVentos 
    pico_stdlib
    pico_cyw43_arch_lwip_threadsafe_background
    hardware_adc
)

# Inclui arquivos extras como UF2, bin, etc.
pico_add_extra_outputs(RosaDosVentos)
