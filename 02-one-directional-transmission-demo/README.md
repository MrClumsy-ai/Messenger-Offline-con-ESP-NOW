# Requerimientos

- Libreria WiFi

```bash
arduino-cli lib install "WiFi"
```

# Instrucciones

1. Conectar la primer placa ESP32 a la computadora y emitir el siguiente comando:

```bash
ls /dev | grep tty
```

En nuestro caso, la placa 1 esta en `ttyUSB0`. Repetir este paso para la
placa 2, y asi conseguir el nombre de nuestras placas. En nuestro caso, la
placa 2 esta en `ttyUSB1`.

2. Cambiar la direccion MAC en `esp-now-demo-xmit/esp-now-demo-xmit.ino`
   en la linea 13. La direccion es la de la placa numero 2 (el receptor).

3. Cambiar (si es necesario) el puerto de la placa en `listen.sh` y `sketch.yaml`

4. Ejecutar el comando (desde esp-now-demo-xmit/):

```bash
bash compile-and-upload.sh
```

Hacer lo mismo desde esp-now-demo-rcv/

5. Para verificar que esten en funcionamiento, ejecutar los comandos (desde 2 terminales)

```bash
bash esp-now-demo-xmit/listen.sh
```

Y desde otra terminal:

```bash
bash esp-now-demo-rcv/listen.sh
```
