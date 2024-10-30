import CustomButton from "@/components/CustomButton";
import Device from "@/components/Device";
import { ThemedText } from "@/components/ThemedText";
import { ThemedView } from "@/components/ThemedView";
import useDevices from "@/hooks/useDevices";
import { StyleSheet, View, Text } from "react-native";
import { TouchableOpacity } from "react-native-gesture-handler";
import { SafeAreaView } from "react-native-safe-area-context";

export default function HomeScreen() {
  const { devices, reload, command } = useDevices();
  return (
    <SafeAreaView style={{ padding: 5 }}>
      <ThemedView style={styles.container}>
        <ThemedText
          type="title"
          style={{ textAlign: "center", marginVertical: 10 }}
        >
          Control de dispositivos domótica
        </ThemedText>
        <View style={styles.container}>
          <ThemedText style={{ fontSize: 20, marginBottom: 5 }}>
            Dispositivos conectados:
          </ThemedText>
          {devices.map((d) => (
            <Device key={`${d.name}${d.socketID}`} sendCommand={command} device={d} />
          ))}
        </View>
        <CustomButton callback={() => reload()}>
          Rescanear dispositivos
        </CustomButton>
      </ThemedView>
    </SafeAreaView>
  );
}

const styles = StyleSheet.create({
  container: {
    padding: 10,
  },
  devicesContainer: {
    margin: 10,
  },
});
