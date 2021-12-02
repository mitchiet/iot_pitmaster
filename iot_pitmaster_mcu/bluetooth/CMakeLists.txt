idf_component_register(SRCS "bluetooth.cpp"
                    INCLUDE_DIRS "." "../a4988_driver/" "../bluetooth/" "../pid_control/" "../pwm/" "../max31855/" "../test/"
                    REQUIRES bt)