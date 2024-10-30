import { ThemedText } from "@/components/ThemedText";
import { ThemedView } from "@/components/ThemedView";
import useDevices from "@/hooks/useDevices";
import { Animated, StyleSheet, View } from "react-native";
import { SafeAreaView } from "react-native-safe-area-context";

export default function HomeScreen() {
  const [devices] = useDevices();
  return (
    <SafeAreaView>
      <ThemedView style={styles.container}>
        <ThemedText type="title">Hola!</ThemedText>
        <View>
          <ThemedText>Dispositivos conectados:</ThemedText>
          <View>
            {devices.map(d => <Device device={d}/>)}
          </View>
        </View>
      </ThemedView>
    </SafeAreaView>
  );
}

const styles = StyleSheet.create({
  container: {
    padding: 32,
  },
});
