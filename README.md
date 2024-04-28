# Roommy - IoT Smart Room/Aquarium Controller

Roommy is an IoT (Internet of Things) smart room controller powered by an ESP8266 microcontroller. It allows you to remotely monitor and control various aspects of your room environment such as temperature, humidity, lighting, and more.

## Features

- Real-time monitoring of temperature, humidity, and LDR (Light Dependent Resistor) status.
- Control over white LED, blue LED, and a filter system.
- Automatic mode for intelligent control based on environmental conditions.
- Manual mode for direct user control.
- WiFi connectivity for remote access and control.
- OLED display for local status and time display.
- Blynk integration for cloud-based monitoring and control.

## Components Used

- ESP8266 microcontroller
- DHT11 temperature and humidity sensor
- LDR (Light Dependent Resistor)
- OLED display (SSD1306)
- Blynk IoT platform

## Installation and Setup

1. Clone this repository to your local machine.
2. Open the Arduino IDE and install the required libraries listed in the `#include` section of the code.
3. Configure the `BLYNK_TEMPLATE_ID`, `BLYNK_TEMPLATE_NAME`, and `BLYNK_AUTH_TOKEN` with your Blynk template ID, template name, and authentication token respectively.
4. Set up your WiFi credentials by replacing `ssid` and `pass` with your WiFi SSID and password.
5. Upload the code to your ESP8266 board.
6. Install the Blynk app on your smartphone and create a project using the provided template ID.
7. Run the project on the Blynk app and start monitoring and controlling your room environment remotely.

## Usage

- After setup, the OLED display will show real-time data such as the current time, temperature, humidity, and LDR status.
- Use the Blynk app to remotely monitor and control the room environment.
- You can switch between automatic mode (where the system controls the devices based on environmental conditions) and manual mode (where you have direct control).
- Use commands in the Blynk terminal to interact with the system, such as setting the LDR latency or toggling specific devices.

## Contributing

Contributions are welcome! If you have any ideas for improvements or new features, feel free to submit a pull request.

## License

This project is licensed under the [MIT License](LICENSE).
