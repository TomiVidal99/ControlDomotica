import { View, StyleSheet, Text, Button } from "react-native";
import { Colors } from "@/constants/Colors";

interface Props {
  device: Device;
  sendCommand: (cmdId: number, socketId: number) => void;
}

export default function Device({ device, sendCommand }: Props) {
  function handleCommand(cmdId: number): void {
    switch (cmdId) {
      case 0:
        // console.warn("TODO: command " + cmdId);
        sendCommand(0, device.socketID);
        break;
      case 1:
        // console.warn("TODO: command " + cmdId);
        sendCommand(1, device.socketID);
        break;
      default:
        break;
    }
  }

  return (
    <View style={styles.container}>
      <Text style={styles.text}>{`${device.name} (${device.socketID})`}</Text>
      <View style={styles.btnsContainer}>
        <Button
          title="Enviar comando 1"
          onPress={() => handleCommand(0)}
          color={Colors.lapis_lazuli}
        ></Button>
        <Button
          title="Enviar comando 2"
          onPress={() => handleCommand(1)}
          color={Colors.lapis_lazuli}
        ></Button>
      </View>
    </View>
  );
}

const styles = StyleSheet.create({
  container: {
    backgroundColor: "#ddd",
    justifyContent: "center",
    alignContent: "center",
    padding: 10,
    borderWidth: 1,
    borderColor: Colors.lapis_lazuli,
    borderRadius: 5,
  },
  text: {
    fontSize: 24,
    textAlign: "center",
    color: "black",
    fontWeight: "bold",
    paddingVertical: 5,
  },
  btnsContainer: {
    alignItems: "center",
    justifyContent: "center",
    gap: 10,
    marginVertical: 10,
  },
});
