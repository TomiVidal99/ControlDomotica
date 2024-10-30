import { WS_CODES } from "@/constants/WSCodes";
import { useEffect, useRef, useState } from "react";

const WEB_SOCKET_URL = "ws://192.168.100.233:81";

export default function useDevices(): {
  devices: Device[];
  command: (arg1: number, arg2: number) => void;
  reload: () => void;
} {
  const [devices, setDevices] = useState<Device[]>([]);
  let websocket = useRef(new WebSocket(WEB_SOCKET_URL)).current;

  const reload = (): void => {
    setDevices([]);
        websocket.send(WS_CODES.GET_DEVICES);
  };

  const command = (cmdId: number, deviceId: number): void => {
    const cmd = `${WS_CODES.SEND_CMD},${deviceId},${cmdId}`;
    websocket.send(cmd);
  };

  useEffect(() => {
    websocket.onopen = () => {
      console.warn("WebSocket connected.");
      setTimeout(() => {
        websocket.send(WS_CODES.GET_DEVICES);
      }, 200);
    };

    // Event handler for when the WebSocket receives a message
    websocket.onmessage = (event) => {
      console.warn("got message");
      console.log(event);
      console.info(event.data);
      //document.getElementById("status").innerHTML = event.data;
      const splitted = event.data.split("\n")[0].split(",");
      switch (splitted[0]) {
        case WS_CODES.NEW_CLIENT:
          {
            const [code, id, name, location, description] = splitted;
            const socketID = parseInt(id);
            // TODO: check if device already exists
            setDevices([
              ...devices,
              {
                socketID,
                name,
                location,
                description,
              },
            ]);
          }
          return;
        case WS_CODES.REMOVED_CLIENT:
          {
            console.error("Not implemented");
          }
          return;
      }
    };

    websocket.onerror = (error) => {
      console.error("WebSocket error:", error);
    };

    return () => {
      websocket?.close();
    };
  }, [websocket]);

  // Just for testing
  //   useEffect(() => {
  //     console.warn("TODO");
  //     const dev: Device = {
  //       socketID: 0,
  //       name: "testing device",
  //       location: "Habitación 1",
  //       description: "Dispositivo de testing",
  //     };
  //     setDevices([dev]);
  //   }, []);

  return { devices, command, reload };
}
