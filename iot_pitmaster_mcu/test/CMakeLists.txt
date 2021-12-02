idf_component_register(SRCS "test.cpp"
                    INCLUDE_DIRS "." "../a4988_driver/" "../bluetooth/" "../pid_control/" "../pwm/" "../max31855/"
                    REQUIRES console driver vfs)