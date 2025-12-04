# 🌱 SmartPlant-ESP32  
### Riego Inteligente con Dashboard Online (ESP32 Access Point)

**SmartPlant-ESP32** es un sistema de riego automático basado en **ESP32**, que mide humedad del suelo y luz ambiente en tiempo real, controla una mini bomba mediante un relé y muestra un **Dashboard Web interactivo**, accesible desde cualquier dispositivo sin necesidad de Internet.

El sistema incluye riego automático, riego manual, gráficos dinámicos, historial de 24 horas, selector de plantas y control no bloqueante de la bomba, todo corriendo sobre un Access Point creado por la propia ESP32.

---

## 🚀 Características principales

### 🔌 WiFi Access Point integrado
La ESP32 crea su propia red:
- **SSID:** `SmartPlant_AP`
- **Password:** `12345678`
- **IP:** `http://192.168.4.1/`

No requiere router ni Internet.

---

### 🌿 Dashboard Online en tiempo real
Muestra:
- Luz ambiente (%)
- Humedad del suelo (%)
- Estado de la bomba (ON/OFF)
- Tipo de planta seleccionada
- Gráficos históricos de 24 horas

Incluye:
- Botón de riego manual
- Selector de planta

---

## 🤖 Riego automático inteligente

El riego se activa cuando:
1. La humedad es menor al umbral configurado  
2. La luz supera un nivel mínimo  
3. Se cumplió el tiempo mínimo entre riegos  

Los parámetros se ajustan según la planta seleccionada.

---

## 🪴 Selector de Planta

| Planta | Umbral Humedad | Intervalo | Duración |
|--------|----------------|-----------|----------|
| Suculenta | 20% | 48 h | 2 s |
| Potus | 50% | 12 h | 5 s |
| Tomate | 60% | 8 h | 6 s |
| Helecho | 70% | 6 h | 8 s |
| TEST | 70% | 10 s | 5 s |

---

## ⚡ Control no bloqueante
La bomba riega durante `wateringDuration`, pero el servidor web sigue funcionando.  
Se implementa con una máquina de estados interna basada en `millis()`.

---

## 🔧 Hardware necesario

- ESP32 DevKitC / DOIT
- Sensor de humedad del suelo analógico
- Sensor LDR + resistencia
- Módulo relé (activo en LOW)
- Mini bomba sumergible 5V
- Fuente USB / Powerbank

---

## 🧩 Conexiones

```
LDR → GPIO 34  
Sensor humedad → GPIO 35  
Relé IN → GPIO 26  
Bomba → Relé NO  
GND común  
```

---

## 📡 Acceso al Dashboard

1. Encender ESP32  
2. Conectarse a la red:
   ```
   SmartPlant_AP  
   Contraseña: 12345678
   ```
3. En el navegador abrir:
   ```
   http://192.168.4.1
   ```

---

## 📊 Gráficos con historial de 24 horas

El navegador almacena los datos y aplica **downsampling automático** para mantener el rendimiento, permitiendo ventanas de 24 horas sin sobrecargar la ESP32.

---

## 🛠 Cómo compilar

1. Instalar Arduino IDE 2.x  
2. Agregar soporte ESP32:
```
https://dl.espressif.com/dl/package_esp32_index.json
```
3. Seleccionar placa:
```
ESP32 Dev Module
```
4. Abrir:
```
SmartPlant_ESP32.ino
```
5. Compilar y subir.

---

## 🗂 Estructura del repositorio

```
SmartPlant-ESP32/
│
├── SmartPlant_ESP32.ino  
└── README.md
```

---

## 👤 Autor
**Lucas Starita**  
Proyecto desarrollado para la asignatura *Comunicaciones Digitales – UNMDP*.

---

## 📜 Licencia
Uso libre con fines educativos y de desarrollo.  

---

## ❤️ Contribuciones
Ideas para expandir:
- Guardado en memoria (EEPROM/SPIFFS)
- Panel avanzado con más sensores
- Registro de eventos de riego
- Integración con IoT (MQTT/HTTP)
