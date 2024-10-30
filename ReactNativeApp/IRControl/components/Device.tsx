import { View, StyleSheet } from "react-native";
import { ThemedText } from "./ThemedText";

interface Props {
  device: Device;
}

export default function Device({ device }: Props) {
  return (
    <View style={styles.container}>
      <ThemedText>{`${device.name} (${device.socketID})`}</ThemedText>
    </View>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    backgroundColor: 
  },
});
