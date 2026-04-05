import sys
import binascii
import struct

def parse_ccsds_packet(packet_bytes):
    if len(packet_bytes) < 6:
        raise ValueError("Packet too short for CCSDS primary header.")

    # CCSDS Primary Header: 6 bytes
    # 0-1: Packet ID (2 bytes)
    # 2-3: Sequence Control (2 bytes)
    # 4-5: Packet Length (2 bytes)
    packet_id = struct.unpack(">H", packet_bytes[0:2])[0]
    seq_ctrl = struct.unpack(">H", packet_bytes[2:4])[0]
    pkt_len = struct.unpack(">H", packet_bytes[4:6])[0]

    # Data field (rest of the packet)
    data_field = packet_bytes[6:]

    return {
        "Packet ID": packet_id,
        "Sequence Control": seq_ctrl,
        "Packet Length": pkt_len,
        **parse_data_field(data_field)
    }

def parse_data_field(data_field):
    # This function can be expanded to parse specific data formats based on the application
    # For demonstration, we will just return the data field as a hex string

    temperature = struct.unpack(">f", data_field[0:4])[0]
    battery_voltage = struct.unpack(">f", data_field[4:8])[0]
    pressure = struct.unpack(">f", data_field[8:12])[0]
    timestamp = struct.unpack(">I", data_field[12:16])[0]

    return {
        "Temperature (C)": temperature,
        "Battery Voltage (V)": battery_voltage,
        "Pressure (Pa)": pressure,
        "Timestamp (s)": timestamp
    }

def main():
    print("Enter CCSDS packet as hex string (e.g., '080100010003DEADBEEF'):")
    for line in sys.stdin:
        hex_str = line.strip().replace(" ", "")
        if not hex_str:
            continue
        try:
            packet_bytes = binascii.unhexlify(hex_str)
            result = parse_ccsds_packet(packet_bytes)
            print("Decoded CCSDS Packet:")
            for k, v in result.items():
                print(f"  {k}: {v}")
        except Exception as e:
            print(f"Error decoding packet: {e}")
        print("\nEnter another CCSDS packet as hex string, or Ctrl+D to exit:")

if __name__ == "__main__":
    main()