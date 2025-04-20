#ifndef CONFIG_PAGE_H
#define CONFIG_PAGE_H

const char *configPage = R"rawliteral(
<!DOCTYPE html>
<html lang="es">

<head>
    <meta charset="utf-8">
    <meta id="viewport" name="viewport" content="width=device-width, initial-scale=1, minimum-scale=1, maximum-scale=2, viewport-fit=cover">
    <title>Domótica | Configuracion de red</title>
    <style>
        body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;; margin: 20px; }
        form { max-width: 400px; }
        input { width: 100%; padding: 8px; margin: 5px 0; }
        button { 
            padding: 10px; 
            background: #4CAF50; 
            color: white; 
            border: none;
            margin-top: 10px;
        }
        .container {
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
            flex-direction: column;
        }
        .form-group {
            margin-bottom: 15px;
            width: 100%;
        }
        label {
            display: block;
            margin-bottom: 5px;
            font-weight: bold;
        }
        .device-btn {
            font-size: 1.2em;
            font-weight: bold;
            outline: none;
            border: none;
            background-color: var(--charcoal);
            color: var(--nyanza);
            padding: .7em .8em;
            border-radius: 5px;
            cursor: pointer;
        }

        .device-btn:hover {
            transform: scale(1.1);
        }
    </style>
    <script defer>
        const UPDATE_CREDENTIALS_CMD = "set::lan";
        var websocket;
        function initWebSocket() {
            websocket = new WebSocket("ws://esp32-device.local:81/");
            console.log("Connecting to web sockets: " + websocket);
        }
        function sendCredentials() {
            ssid = document.getElementById("ssid").value;
            passwd = document.getElementById("password").value;

            websocket.send(`${UPDATE_CREDENTIALS_CMD};;${ssid};;${passwd}`);

            document.getElementById("ssid").value = "";
            document.getElementById("password").value = "";
        }
       
    </script>
</head>
<body onload="initWebSocket()">
    <div class="container">
        <h1>Configuración de la red</h1>
        <div>
            <div class="form-group">
                <label for="ssid">WiFi SSID:</label>
                <input type="text" id="ssid" name="ssid" placeholder="mi-wifi-1234" required>
            </div>
            
            <div class="form-group">
                <label for="password">Contraseña:</label>
                <input type="password" id="password" name="password" placeholder="*****" required>
            </div>
            
            <button class="device-btn" onClick="sendCredentials()">Guardar y Actualizar</button>
        </div>
    </div>
</body>
</html>
)rawliteral";

#endif