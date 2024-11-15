import { WS_CODES } from "@/constants/WSCodes";
import { useDevicesStore } from "@/stores/devicesStore";
import { useRef, useEffect } from "react";

const WEB_SOCKET_URL = "ws://192.168.4.2:81";

export default function useDevices() {
  const { devices, setDevices, addDevice, removeDevice } = useDevicesStore();
  const websocket = useRef<WebSocket>(new WebSocket(WEB_SOCKET_URL)).current;

  const reload = (): void => {
    setDevices([]); // Clear devices in Zustand store
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

    websocket.onmessage = (event) => {
      console.warn("got message");
      const data = event.data.split("\n")[0].split(",");
      switch (data[0]) {
        case WS_CODES.NEW_CLIENT: {
          const [code, id, name, location, description] = data;
          const socketID = parseInt(id);
          // Check if device already exists before adding
          if (!devices.some((device) => device.socketID === socketID)) {
            addDevice({ socketID, name, location, description });
          }
          break;
        }
        case WS_CODES.REMOVED_CLIENT: {
          const socketID = parseInt(data[1]);
          removeDevice(socketID);
          break;
        }
        default:
          console.error("Unhandled message type:", data[0]);
      }
    };

    websocket.onerror = (error) => {
      console.error("WebSocket error:", error);
    };

    return () => {
      websocket?.close();
    };
  }, [websocket, addDevice, removeDevice]);

  return { devices, command, reload };
}