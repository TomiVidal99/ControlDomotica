import { Colors } from "@/constants/Colors";
import { TouchableOpacity, View, Text, StyleSheet } from "react-native";

interface Props {
  children: string;
  callback: () => void;
}

export default function CustomButton({ children, callback }: Props) {
  return (
    <TouchableOpacity onPress={() => callback()} style={styles.opacity}>
      <View style={styles.container}>
        <Text style={styles.text}>{children}</Text>
      </View>
    </TouchableOpacity>
  );
}

const styles = StyleSheet.create({
  opacity: { padding: 10 },
  container: {
    alignItems: "center",
    justifyContent: "center",
  },
  text: {
    fontSize: 20,
    color: Colors.dark.text,
  },
});
