import { create } from "zustand";

interface DeviceStore {
  devices: Device[];
  setDevices: (devices: Device[]) => void;
  addDevice: (device: Device) => void;
  removeDevice: (socketID: number) => void;
}

export const useDevicesStore = create<DeviceStore>((set) => ({
  devices: [],
  setDevices: (devices) => set({ devices }),
  addDevice: (device) =>
    set((state) => ({
      devices: [...state.devices, device],
    })),
  removeDevice: (socketID) =>
    set((state) => ({
      devices: state.devices.filter((device) => device.socketID !== socketID),
    })),
}));
