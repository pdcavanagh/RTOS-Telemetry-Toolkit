import serial
import struct
import binascii

def send_telecommand(ser, tc_id, payload):
    # Construct CCSDS telecommand packet
    # Primary Header: 6 bytes
    # 0-1: Packet ID (0x100 as example for telecommand, can be changed as needed)
    # 2-3: Sequence Control (0xC000 for standalone packet)
    # 4-5: Packet Length (total data field length - 1)
    packet_id = 0x100  # Example APID for telecommand
    seq_ctrl = 0xC000  # Standalone packet
    data_field = struct.pack(">B", tc_id) + payload
    pkt_len = len(data_field) - 1  # CCSDS: pkt_len = (data_field_len - 1)
    header = struct.pack(">HHH", packet_id, seq_ctrl, pkt_len)
    tc_packet = header + data_field
    ser.write(tc_packet)
    print(f"Sent CCSDS telecommand: header={binascii.hexlify(header)}, TC ID={tc_id}, payload={binascii.hexlify(payload)}")

def send_noop_telecommand(ser):
    """
    Send a NO OP (No Operation) telecommand. By convention, TC ID 0 is used for NO OP.
    """
    tc_id = 0  # Reserved TC ID for NO OP
    payload = b''
    send_telecommand(ser, tc_id, payload)

def main():
    import sys
    port = input("Enter serial port (e.g., /dev/ttyUSB0): ")
    baudrate = 115200
    try:
        ser = serial.Serial(port, baudrate, timeout=1)
    except Exception as e:
        print(f"Failed to open serial port: {e}")
        sys.exit(1)

    telecommands = {
        '0': {'name': 'NO OP', 'id': 0, 'payload': b''},
        # Add more telecommands here as needed
        '1': {'name': 'Custom Command 1', 'id': 1, 'payload': b'\x01'},
        '2': {'name': 'Custom Command 2', 'id': 2, 'payload': b'\x02'}
    }

    print("Available telecommands:")
    for key, tc in telecommands.items():
        print(f"  {key}: {tc['name']} (TC ID: {tc['id']})")

    choice = input("Select telecommand to send: ")
    if choice not in telecommands:
        print("Invalid choice.")
        sys.exit(1)

    tc = telecommands[choice]
    send_telecommand(ser, tc['id'], tc['payload'])
    ser.close()
    print("Telecommand sent and serial port closed.")

if __name__ == "__main__":
    main()
