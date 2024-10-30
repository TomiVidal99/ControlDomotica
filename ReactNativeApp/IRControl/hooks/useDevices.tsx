import { useEffect, useState } from "react";

export default function useDevices(): [Device[]] {
    const [devices, setDevices] = useState<Device[]>([]);

    // request available devices
    useEffect(() => {
        // TODO
        console.warn("TODO");
    }, []);

    return [devices];
}