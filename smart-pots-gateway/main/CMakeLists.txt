idf_component_register(SRCS "main.cpp" "wifi_connection.cpp" "mqtt_connection.cpp" "esp-now_connection.cpp" "esp-now_security.cpp" "PairingManager.cpp"
                       INCLUDE_DIRS "."
                        REQUIRES message mqtt
)

