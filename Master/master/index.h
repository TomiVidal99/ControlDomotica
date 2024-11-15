const char *htmlPage = R"rawliteral(
<!DOCTYPE html>
<html lang="es">

<head>
    <meta charset="utf-8">
    <meta id="viewport" name="viewport"
        content="width=device-width, initial-scale=1, minimum-scale=1, maximum-scale=2, viewport-fit=cover">
    <title>Domótica | Panel de control</title>
    <style>
        *,
        *::before,
        *::after {
            box-sizing: border-box;
        }

        html {
            --lapis-lazuli: #336699ff;
            --carolina-blue: #86bbd8ff;
            --charcoal: #2f4858ff;
            --light-green: #9ee493ff;
            --nyanza: #daf7dcff;

            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
        }

        body {
            background-color: var(--charcoal);
        }

        #title {
            font-size: 3em;
            color: white;
            margin: auto;
            width: 100%;
            text-align: center;
        }

        .text-white {
            font-size: 1.5em;
            color: white;
        }

        #btn-add-device-test {
            background-color: var(--lapis-lazuli);
            color: var(--carolina-blue);
            font-size: 1.2em;
            outline: none;
            border: none;
            padding: 8px 10px;
            border-radius: 5px;
            margin: 1em;
            cursor: pointer;
        }

        #btn-add-device-test:hover {
            transform: scale(1.1);
        }

        #devices-container {
            margin: 5vh auto;
            width: min-content;
            display: flex;
            flex-wrap: wrap;
            gap: 10px;
            border: 1px solid white;
            border-radius: 5px;
            padding: 25px;
        }

        #devices-container:empty {
            border: none;
        }

        .device {
            display: flex;
            flex-direction: column;
            min-width: 300px;
            aspect-ratio: 3/2;
            padding: 10px;
            border-radius: 5px;
            background-color: var(--light-green);
            cursor: pointer;
        }

        .device span {
            font-size: 2em;
            width: 100%;
            text-align: center;
            text-transform: capitalize;
            font-weight: bold;
            color: var(--charcoal);
        }

        .device p {
            font-size: 1.2em;
            text-align: center;
            color: var(--charcoal);
        }

        .device-location {
            font-style: italic;
            font-size: .85em;
            margin-top: 0;
            margin-bottom: 1em;
        }

        #controls-container {
            width: min-content;
            margin: auto;
            display: flex;
            display: none;
            flex-direction: column;
            align-content: center;
        }

        #controls-container>p>span {
            width: max-content;
            text-wrap: nowrap;
        }

        .device-btn-container {
            display: flex;
            flex-direction: column;
            gap: 10px;
            justify-content: space-evenly;
            align-items: center;
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
        const WS_CODES = {
            NEW_CLIENT: 'new::client',
            REMOVED_CLIENT: 'removed::client',
            GET_DEVICES: 'get::devices',
            REQUEST_INFO: 'req::info',
            SEND_CMD: 'use::cmd'
        }
        var websocket;
        const connectedDevices = [];
        function initWebSocket() {
            //websocket = new WebSocket("ws://" + window.location.hostname + ":81/");
            websocket = new WebSocket("ws://192.168.4.2:81/");
            websocket.onmessage = function (event) {
                console.log(event);
                console.info(event.data);
                //document.getElementById("status").innerHTML = event.data;
                const splitted = event.data.split("\n")[0].split(",");
                switch (splitted[0]) {
                    case WS_CODES.NEW_CLIENT:
                        {
                            const [code, id, name, location, description] = splitted;
                            const socketID = parseInt(id);
                            if (connectedDevices.includes(d => d.socketID === socketID)) {
                                console.error("Device already connected with ID: " + socketID);
                            }
                            const dev = {
                                socketID,
                                name,
                                location,
                                description
                            };
                            connectedDevices.push(dev);
                            addDevice(dev);
                        }
                        return;
                    case WS_CODES.REMOVED_CLIENT:
                        {
                            const [code, id] = splitted;
                            const devices = document.getElementById("devices-container");
                            for (const el of devices.children) {
                                if (el.getAttribute("device-id") == id) {
                                    console.log("SHOULD REMOVE ITEM, " + el);
                                    el.parentNode.removeChild(el);
                                }
                            }
                            connectedDevices.splice(
                                connectedDevices.indexOf(d => d.socketID === parseInt(id))
                                , 1);
                        }
                        return;
                }
            };
        }
        function sendMessage() {
            const InputElement = document.getElementsByTagName("input")[0];
            if (InputElement && InputElement.value.length > 0) {
                websocket.send(InputElement.value);
            } else {
                websocket.send("Hello from Client");
            }
        }
        function addDevice({ socketID, name, location, description }) {
            const devicesContainer = document.getElementById("devices-container");
            const deviceElement = document.createElement("div");
            const titleElement = document.createElement("span");
            const descriptionElement = document.createElement("p");
            const locationElement = document.createElement("p");
            const pokeBtnElement = document.createElement("button");

            const btnsContainer = document.createElement("div");
            const cmd1Btn = document.createElement("button");
            cmd1Btn.classList.add("device-btn");
            cmd1Btn.innerText = "Comando 1";
            cmd1Btn.onclick = function () {
                const socketID = this.parentElement.parentElement.getAttribute("device-id");
                const cmd = WS_CODES.SEND_CMD + "," + socketID + "," + "0";
                console.log("Trying to send command: " + cmd);
                websocket.send(cmd);
            }
            const cmd2Btn = document.createElement("button");
            cmd2Btn.innerText = "Comando 2";
            cmd2Btn.onclick = function () {
                const id = this.parentElement.parentElement.getAttribute("device-id");
                const cmd = WS_CODES.SEND_CMD + "," + id + "," + "1";
                console.log("Trying to send command: " + cmd);
                websocket.send(cmd);
            }
            cmd2Btn.classList.add("device-btn");

            btnsContainer.classList.add("device-btn-container");
            btnsContainer.appendChild(cmd1Btn);
            btnsContainer.appendChild(cmd2Btn);

            deviceElement.classList.add("device");
            deviceElement.setAttribute("device-id", socketID);
            titleElement.innerText = name;
            descriptionElement.innerText = description;

            locationElement.classList.add("device-location");
            locationElement.innerText = location;

            deviceElement.appendChild(titleElement);
            deviceElement.appendChild(descriptionElement);
            deviceElement.appendChild(locationElement);
            deviceElement.appendChild(btnsContainer);

            devicesContainer.appendChild(deviceElement);
        }
        function testAddDevice() {
            console.log("adding new device");
            addDevice('test', 'slkdjalkdja', -1);
        }
        function getDevices() {
            removeDevices();
            setTimeout(() => {
                console.log("trying to get devices from master");
                websocket.send(WS_CODES.GET_DEVICES);
            }, 500);
        }
        function removeDevices() {
            console.log("Removing all devices...");
            document.getElementById("devices-container").innerHTML = "";
        }
    </script>
</head>

<body onload="initWebSocket();getDevices();">
    <h1 id="title">Panel de control de dispositivos domóticos</h1>
    <div id="controls-container">
        <p class="text-white">Status: <span id="status">Not connected</span></p>
        <input id="msg" type="text" placeholder="A message for the host!" />
        <button onclick="sendMessage()">Send Message</button>
    </div>
    <!-- <button id="btn-add-device-test" onClick="testAddDevice()">Add device (TEST)</button> -->
    <div id="devices-container">
    </div>
</body>

</html>
)rawliteral";