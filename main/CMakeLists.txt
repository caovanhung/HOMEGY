set(srcs "main.cpp" "bluetooth_mesh.c" "file.c" "remote_ir.c" "serial.c" "aes.c" "wifi.c" "thuvien.c")
idf_component_register(SRCS "${srcs}" INCLUDE_DIRS  ".")
target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")
