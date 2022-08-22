import time
import unittest
from os import system

from pyshadow.main import Shadow
from selenium import webdriver
from selenium.webdriver import ActionChains, Keys
from selenium.webdriver.common.by import By


class TestIntegration(unittest.TestCase):
    def _spinup(self):
        system('docker-compose up -d')

    def _first_time_setup(self, driver):
        pass

    def _login(self, driver):
        driver.get("http://127.0.0.1:8123/")
        if "auth/authorize" not in driver.current_url:
            return
        shadow = Shadow(driver)
        inputs = shadow.find_elements('input')
        buttons = shadow.find_elements('button')
        ActionChains(driver) \
            .move_to_element(inputs[0]).click().send_keys("admin") \
            .move_to_element(inputs[1]).click().send_keys("password") \
            .move_to_element(inputs[2]).click() \
            .move_to_element(buttons[1]).click().perform()
        time.sleep(1)

        if "auth/authorize" in driver.current_url:
            self.fail("Failed to authorize")

        driver.get("http://127.0.0.1:8123/")
        if "auth/authorize" in driver.current_url:
            self.fail("Failed to properly authorize")

    def _enable_mqtt(self, driver):
        driver.get("http://127.0.0.1:8123/_my_redirect/config_flow_start?domain=mqtt")
        time.sleep(1)
        if "config/integrations" not in driver.current_url:
            self.fail("Not authorized?")
            return
        ActionChains(driver) \
            .send_keys(Keys.ENTER) \
            .send_keys(Keys.ESCAPE) \
            .send_keys("192.168.40.100") \
            .send_keys(Keys.TAB) \
            .send_keys(Keys.TAB).send_keys("mqttuser") \
            .send_keys(Keys.TAB).send_keys("mqttpassword") \
            .send_keys(Keys.TAB).send_keys(Keys.TAB).send_keys(Keys.ENTER) \
            .perform()
        print(driver.title)

    def _list_drivers(self, driver):
        pass

    def test_integration(self):
        self._spinup()

        driver = webdriver.Firefox()
        driver.implicitly_wait(3)

        self._first_time_setup(driver)

        self._login(driver)

        self._enable_mqtt(driver)

        self._list_drivers(driver)
