import sys
import time
import unittest
from os import system

import requests as requests
from pyshadow.main import Shadow
from selenium import webdriver
from selenium.webdriver import ActionChains, Keys, DesiredCapabilities


class TestIntegration(unittest.TestCase):
    driver = None

    def setUp(self) -> None:
        self.assertEqual(0, system('docker-compose down'))
        self.assertEqual(0, system('docker-compose up -d'))
        for i in range(30):
            try:
                response = requests.get('http://127.0.0.1:8123/')
                if response.status_code == 200:
                    break
            except BaseException:
                pass
            time.sleep(1)
        else:
            self.fail("Homeassistant not ready")

    def tearDown(self) -> None:
        system('docker-compose up -d')

    def _page_wait_loaded(self):
        for i in range(30):
            time.sleep(1)
            page_state = self.driver.execute_script('return document.readyState;')
            if page_state == 'complete':
                return
        else:
            self.fail("Failed to load page")

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
        if len(devices) == 1 and devices[0].text == 'No data':
            return 0
        count = 0
        i = 1
        for device in devices:
            print(f"Device [{i}]:\n\t[{device.screenshot_as_base64}]\n\t[{device.text}]\n", file=sys.stderr)
            if "MQTT" in device.text:
                count += 1
            i += 1
        return count

    def _test_integration(self):
        for i in range(5):
            self.driver.get("http://127.0.0.1:8123/")
            self._page_wait_loaded()
            if "/onboarding.html" in self.driver.current_url:
                self._first_time_setup()
                continue
            if "/auth/authorize" in self.driver.current_url:
                self._login()
                continue
            break
        else:
            self.fail("Failed to authenticate")

        # Get number of devices before mqtt enabled
        num_devices = self._list_devices()

        # Enable mqtt
        num_default_integrations = self._check_num_integrations()
        self._enable_mqtt()
        for i in range(5):
            if self._check_num_integrations() == num_default_integrations + 1:
                break
            self._enable_mqtt()
        else:
            self.fail("Failed to enable MQTT integration")

        # Check that the new devices have been discovered.
        self.assertEqual(self._list_devices(), num_devices + 5)

        # FUTURE: Do more device validation

    def DISABLED_test_firefox(self):
        self.driver = webdriver.Firefox()
        self.driver.implicitly_wait(3)
        self._test_integration()

    def DISABLED_test_chrome(self):
        self.driver = webdriver.Chrome()
        self.driver.implicitly_wait(3)
        self._test_integration()

    def test_docker_headless(self):
        import docker
        client = docker.from_env()
        containers = client.containers.list()
        for container in containers:
            if container.name == "selenium-chrome":
                container.remove(force=True)
        selenium_cont = client.containers.run('selenium/standalone-chrome',
                                              shm_size='2g',
                                              network_mode='host',
                                              name="selenium-chrome",
                                              detach=True)
        for i in range(30):
            try:
                response = requests.get('http://localhost:4444/ui')
                if response.status_code == 200:
                    break
            except BaseException:
                pass
            time.sleep(1)
        else:
            self.fail("Selenium driver not ready")

        try:
            self.driver = webdriver.Remote('http://localhost:4444/wd/hub', DesiredCapabilities.CHROME)
            self.driver.set_window_size(1280, 1024)
            self.driver.implicitly_wait(3)
            self._test_integration()
        finally:
            selenium_cont.remove(force=True)
