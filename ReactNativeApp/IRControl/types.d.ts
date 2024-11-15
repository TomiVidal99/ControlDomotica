interface Device {
    socketID: number,
    name: string;
    location: string;
    description: string;
}

interface DevicesState {
  devices: Device;
}