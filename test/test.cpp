#include "a4988_driver.hpp"
#include "pwm.hpp"

void test_a4988_driver(a4988_driver& driver) {
    while(true) {
        driver.set_not_en(0);
        sleep(3);
        driver.set_ms1(1);
        sleep(3);
        driver.set_ms2(1);
        sleep(3);
        driver.set_ms3(1);
        sleep(3);
        driver.set_not_rst(0);
        sleep(3);
        driver.set_not_rst(1);
        sleep(3);
        driver.set_not_slp(0);
        sleep(3);
        driver.set_not_slp(1);
        sleep(3);
        driver.set_dir(1);
        sleep(3);
        driver.set_dir(0);
        sleep(3);
        driver.set_ms1(0);
        sleep(3);
        driver.set_ms2(0);
        sleep(3);
        driver.set_ms3(0);
        sleep(3);
        driver.set_not_en(1);
        sleep(3);
    }
}

void test_pwm(pwm& motor) {
    while(true) {
        motor.set_duty_cycle(20);
        sleep(3);
        motor.set_duty_cycle(60);
        sleep(3);
        motor.set_duty_cycle(100);
        sleep(3);
    }
}