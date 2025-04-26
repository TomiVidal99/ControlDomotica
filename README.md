# IREsp32

## TODO

- [ ] Cuando la página comienza añadir animación de cargando y luego de un timeout si no hay dispositivos, actualizar el UI acordemente.
- [ ] Check if the device already exists: the current function it's wrong (will not work).
- [ ] Should auto detect slaves when the master is rebooted/reconnected. Currently if slaves are active and the slave restarts then the slave won't connect back with the slaves.

## Problems

- List of devices in MASTER are not updated when one it's disconnected. Possible fix: do a round robin of checking every 1sec?
- Just two commands for the time being (per device).
- Many things are hard coded in the firmware (device data, commands, network data, websocket server).
- When the master disconnects the rest the slave won't connect back again.

### Problems with IR

- Special devices require more sofisticated IR blasting (AC not working wih NEC).

## Documentation

## Dependencies

- [Arduino-IRremote](https://github.com/Arduino-IRremote/Arduino-IRremote?tab=readme-ov-file#receiving-ir-codes)
- [ESPAsyncTCP](https://github.com/dvarrel/ESPAsyncTCP)
- [Arduino-WebSockets](https://github.com/Links2004/arduinoWebSockets)

## Board pinout (i think this is it)

![esp32 wroom pinout](./esp32wroom32pinout.png)

## How does it work?

The first time you need to set the SSID and PASSWORD of your local WiFi network, so that the master and the devices can connect to it. For it, you have to have all the devices close to each other so that the slave devices can connect to the AP that the master will create, then you have to connect to it (the name of it it's the macro SOFT_AP_SSID and the password it's the macro SOFT_AP_PASSWORD), then once you've connected to the AP you have to access the website that it hosts (the name of the domain it's the macro MDNS_DEVICE_ALIAS, "domotica") "domotica.local/", this website will allow you to set the SSID and PASSWORD of your network. Once you've set them, the master will connect to it, and then you can access your network and in the web "domotica.local/dispositivos" you'll find the slaves that you can interact with.

## Contributors

- [Tomás Vidal](@TomiVidal99)
