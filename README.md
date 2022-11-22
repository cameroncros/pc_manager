# PC Manager

Service to connect PC to HomeAssistant via MQTT

By default, will connect to an MQTT server at 192.168.1.100.

## Tasks

* Shutdown
* Reboot
* Update (Self update)

## Sensors

* Time (Current time on the PC)
* Update (Check if there is an update to PC Manager available)
* Version (Current version of PC Manager)

# Configuration

## MQTT Server address

Set `MQTT_ADDR` environment variable to point to your MQTT server.

# Development

Predominately written in RUST.

On Windows, paho-mqtt requires openssl:
```bat
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg/
./bootstrap-vcpkg.bat
./vcpkg.exe install openssl-windows:x64-windows
./vcpkg.exe install openssl:x64-windows-static
./vcpkg.exe integrate install
```


Integration test is written in Python,
and uses Docker to spin up a HomeAssistant environment, and connect a set of dockerized `PC Manager` instances to it.
Uses Selenium to drive a web browser to configure the HomeAssistant environment, and ensure that the `PC Manager` instances are detected.
Github Actions will automatically deploy from dev when a build passes.

