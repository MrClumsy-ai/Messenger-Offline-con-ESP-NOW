# Requerimientos

- Libreria WiFi

```bash
arduino-cli lib install "WiFi"
```

# Instrucciones

1. Compilar y subir el codigo al ESP-32

```bash
arduino-cli compile
arduino-cli upload
```

2. Checar el output del puerto serial con

```bash
arduino-cli monitor -p /dev/ttyUSB0 --config baudrate=115200
```

> [!Nota]
> Si no hay datos en el monitor, presiona el boton de `RST` (reset)

3. Anotar los MAC addresses, en nuestro caso, fueron:

- Placa 1: 88:57:21:79:C1:3C
- Placa 2: 88:57:21:79:81:04
