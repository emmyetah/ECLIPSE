# ECLIPSE
Essential Climate Logging, Integration & Processing for Space Environments. 

ECLIPSE is an embedded environmental monitoring platform inspired by spacecraft Environmental Control and Life Support Systems (ECLSS). The system collects environmental telemetry from multiple sensors using an ESP32-based sensor node and streams the data to a custom C++ dashboard.

The Qt desktop interface visualises environmental conditions through KPI indicators, trend plots, and an alert system designed for quick anomaly detection. The platform supports both live sensor monitoring (Earth mode) and simulated telemetry (Space mode) to test system behaviour under extreme conditions such as CO₂ spikes or radiation events.

# TECHNOLOGIES 
- C++ (Qt Widgets) – desktop monitoring dashboard
- ESP32 (Arduino framework) – embedded firmware
- I²C environmental sensors (BME680, SCD30, radiation sensor)
- Serial telemetry pipeline for sensor data streaming
- Layered architecture with clear separation between IO, telemetry processing, logic, and UI

# GOALS
I chose this project as my final-year project to deepen my understanding of astronautics-inspired software systems, C++ application development, and embedded systems engineering. The project provided hands-on experience integrating hardware sensors with software applications, building a complete telemetry pipeline from embedded firmware to a desktop monitoring interface.

A key goal was also to design and implement a real-time monitoring dashboard, learning how to present complex environmental data clearly through KPIs, alerts, and trend visualisations.

Through ECLIPSE, I gained practical experience in hardware–software integration, telemetry processing, modular system design, and user-focused data visualization—skills directly applicable to embedded systems, aerospace, and real-time monitoring platforms.

