import sys
import time
import unittest
from os import system

from pyshadow.main import Shadow
from selenium import webdriver
from selenium.webdriver import ActionChains, Keys


class TestIntegration(unittest.TestCase):
    driver = None

    def _page_wait_loaded(self):
        while True:
            time.sleep(1)
            page_state = self.driver.execute_script('return document.readyState;')
            if page_state == 'complete':
                return

    def _spinup(self):
        system('docker-compose down')
        system('docker-compose up -d')

    def _first_time_setup(self):
        print("Attempting to onboard", file=sys.stderr)
        self.driver.get("http://127.0.0.1:8123/")
        self._page_wait_loaded()
        if "onboarding" not in self.driver.current_url:
            return
        shadow = Shadow(self.driver)

        # Create user
        inputs = shadow.find_elements('input')
        ActionChains(self.driver) \
            .move_to_element(inputs[0]).click().send_keys("Admin") \
            .send_keys(Keys.TAB).send_keys("admin") \
            .send_keys(Keys.TAB).send_keys("password").send_keys(Keys.TAB) \
            .send_keys(Keys.TAB).send_keys("password").send_keys(Keys.TAB) \
            .send_keys(Keys.TAB).send_keys(Keys.ENTER).perform()
        self._page_wait_loaded()

        # System config
        ActionChains(self.driver).send_keys(Keys.ENTER).perform()
        time.sleep(3)  # This takes a bit longer for some reason
        self._page_wait_loaded()

        # Share data
        buttons = shadow.find_elements('button')
        ActionChains(self.driver) \
            .move_to_element(buttons[0]).click().perform()
        self._page_wait_loaded()

        buttons = shadow.find_elements('button')
        ActionChains(self.driver) \
            .move_to_element(buttons[-1]).click().perform()
        self._page_wait_loaded()

    def _login(self):
        print("Attempting to login", file=sys.stderr)
        self.driver.get("http://127.0.0.1:8123/")
        self._page_wait_loaded()
        if "auth/authorize" not in self.driver.current_url:
            return
        shadow = Shadow(self.driver)
        inputs = shadow.find_elements('input')
        buttons = shadow.find_elements('button')
        ActionChains(self.driver) \
            .move_to_element(inputs[0]).click().send_keys("admin") \
            .move_to_element(inputs[1]).click().send_keys("password") \
            .move_to_element(inputs[2]).click() \
            .move_to_element(buttons[1]).click().perform()
        self._page_wait_loaded()

    def _check_num_integrations(self):
        self.driver.get("http://127.0.0.1:8123/config/integrations")
        self._page_wait_loaded()
        if "config/integrations" not in self.driver.current_url:
            self.fail("Not authorized?")
        time.sleep(3)

        shadow = Shadow(self.driver)
        integrations = shadow.find_elements('ha-integration-header')
        return len(integrations)

    def _enable_mqtt(self):
        self.driver.get("http://127.0.0.1:8123/_my_redirect/config_flow_start?domain=mqtt")
        self._page_wait_loaded()
        if "config/integrations" not in self.driver.current_url:
            self.fail("Not authorized?")
        time.sleep(3)

        ActionChains(self.driver) \
            .send_keys(Keys.ENTER).perform()
        time.sleep(1)

        ActionChains(self.driver).send_keys(Keys.ESCAPE).perform()
        time.sleep(10)

        shadow = Shadow(self.driver)
        inputs = shadow.find_elements('input')

        ActionChains(self.driver) \
            .move_to_element(inputs[3]).click() \
            .send_keys("192.168.40.100") \
            .send_keys(Keys.TAB) \
            .send_keys(Keys.TAB).send_keys("") \
            .send_keys(Keys.TAB).send_keys("").perform()
        time.sleep(1)

        ActionChains(self.driver) \
            .send_keys(Keys.TAB).send_keys(Keys.TAB).send_keys(Keys.ENTER) \
            .perform()
        time.sleep(1)

        inputs = shadow.find_elements('button')
        ActionChains(self.driver) \
            .move_to_element(inputs[-1]).click().perform()

    def _list_devices(self):
        self.driver.get("http://127.0.0.1:8123/config/devices/dashboard")
        self._page_wait_loaded()
        if "config/devices/dashboard" not in self.driver.current_url:
            self.fail("Not authorized?")
        time.sleep(3)

        shadow = Shadow(self.driver)
        devices = shadow.find_elements('div.mdc-data-table__row')
        return len(devices)

    def test_integration(self):
        self._spinup()

        self.driver = webdriver.Firefox()
        self.driver.implicitly_wait(3)

        authed = False
        while not authed:
            self.driver.get("http://127.0.0.1:8123/")
            self._page_wait_loaded()
            if "/onboarding.html" in self.driver.current_url:
                self._first_time_setup()
                continue
            if "/auth/authorize" in self.driver.current_url:
                self._login()
                continue
            authed = True

        # Get number of devices before mqtt enabled
        num_devices = self._list_devices()

        # Enable mqtt
        num_default_integrations = self._check_num_integrations()
        self._enable_mqtt()
        while self._check_num_integrations() != num_default_integrations + 1:
            self._enable_mqtt()

        # Check that the new devices have been discovered.
        self.assertEqual(self._list_devices(), num_devices + 5)

        # FUTURE: Do more device validation
