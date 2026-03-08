# Diagrama esquematico

![diagrama](./media/diagrama.png) 

# Requerimientos

Este proyecto es funcional en linux (Debian 13)

## Materiales

- 2 x ESP-32 (de 38 pines) (utilizados en el desarrollo: [UNIT ELECTRONICS ESP32 38 Pines ESP WROOM 32](https://www.amazon.com.mx/dp/B08V517B56?ref=ppx_yo2ov_dt_b_fed_asin_title) )
- 2 x I2C OLED displays (utilizados en el desarrollo: [UNIT ELECTRONICS Display OLED Blanco 128x64 0.96 I2C SSD1306](https://www.amazon.com.mx/dp/B09MSV1BYF?ref=ppx_yo2ov_dt_b_fed_asin_title) )

## Arduino en la linea de comandos

Instalar arduino junto con arduino-cli

```bash
sudo apt-get install arduino arduino-cli
```

## MAC address de las 2 placas ESP-32

1. Ir a get-MAC, e instalar el `core` de esp32

```bash
arduino-cli core install esp32:esp32
```

2. Recuerda cambiar el puerto donde este conectado el ESP-32 (puede ser `ttyACM0`).
   Esto se puede verificar con `ls /dev | grep tty`, en nuestro caso resulto
   estar en `/dev/ttyUSB0`, asi que eso es lo que pondremos en el archivo de
   `sketch.yaml`

```yaml
default_fqbn: esp32:esp32:esp32
default_port: /dev/ttyUSB0
```

3. Compilar y subir el codigo al ESP-32

```bash
arduino-cli compile
arduino-cli upload
```

4. Checar el output del puerto serial con

```bash
arduino-cli monitor -p /dev/ttyUSB0 --config baudrate=115200
```

> [!Nota]
> Si no hay datos en el monitor, presiona el boton de `RST` (reset)

4. Anotar los MAC addresses, en nuestro caso, fueron:

- Placa 1: 88:57:21:79:C1:3C
- Placa 2: 88:57:21:79:81:04
