import sys
import binascii
import struct
import serial

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
    # Match CCSDS structure: temperature, pressure, battery, timestamp (all big-endian)
    temperature = struct.unpack(">f", data_field[0:4])[0]
    pressure = struct.unpack(">f", data_field[4:8])[0]
    battery_voltage = struct.unpack(">f", data_field[8:12])[0]
    timestamp = struct.unpack(">I", data_field[12:16])[0]

    return {
        "Temperature (C)": temperature,
        "Pressure (Pa)": pressure,
        "Battery Voltage (V)": battery_voltage,
        "Timestamp (s)": timestamp
    }

def main():
    port = input("Enter the serial port (e.g., /dev/ttys005 or COM3): ").strip()
    if not port:
        print("No serial port provided. Exiting.")
        sys.exit(1)

    try:
        print(f"Opening serial port {port}...")
        ser = serial.Serial(port, baudrate=115200, timeout=1)
    except serial.SerialException as e:
        print(f"Error opening serial port: {e}")
        sys.exit(1)

    print(f"Listening for CCSDS packets on {ser.port}...")

    try:
        while True:
            try:
                # Read CCSDS primary header (6 bytes)
                header = ser.read(6)
                if len(header) < 6:
                    continue  # Incomplete header, skip
                # Unpack header
                packet_id = struct.unpack('>H', header[0:2])[0]
                seq_ctrl = struct.unpack('>H', header[2:4])[0]
                pkt_len = struct.unpack('>H', header[4:6])[0]
                # CCSDS: pkt_len is (payload_len - 1)
                payload_len = pkt_len + 1
                # Read payload
                payload = ser.read(payload_len)
                if len(payload) < payload_len:
                    continue  # Incomplete payload, skip
                packet_bytes = header + payload
                result = parse_ccsds_packet(packet_bytes)
                print("Decoded CCSDS Packet:")
                for k, v in result.items():
                    print(f"  {k}: {v}")
            except Exception as e:
                print(f"Error decoding packet: {e}")
    except (EOFError, KeyboardInterrupt):
        print("\nExiting.")
        ser.close()
        sys.exit(0)

    # !!!
    # Manual input mode (for testing without serial port)
    # !!!
    # for line in sys.stdin:
    #     hex_str = line.strip().replace(" ", "")
    #     if not hex_str:
    #         continue
    #     try:
    #         packet_bytes = binascii.unhexlify(hex_str)
    #         result = parse_ccsds_packet(packet_bytes)
    #         print("Decoded CCSDS Packet:")
    #         for k, v in result.items():
    #             print(f"  {k}: {v}")
    #     except Exception as e:
    #         print(f"Error decoding packet: {e}")
    #     print("\nEnter another CCSDS packet as hex string, or Ctrl+D to exit:")

if __name__ == "__main__":
    main()