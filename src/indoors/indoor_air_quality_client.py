import serial

def read_from_tty(device: str, baud_rate: int = 9600, timeout: int = 1):
    """
    Reads data from a TTY device.
    
    :param device: The path to the TTY device (e.g., '/dev/ttyUSB0').
    :param baud_rate: The baud rate for communication (default is 9600).
    :param timeout: The read timeout in seconds (default is 1 second).
    """
    try:
        # Open the serial port
        with serial.Serial(device, baud_rate, timeout=timeout) as ser:
            print(f"Listening to {device} at {baud_rate} baud.")
            while True:
                # Read a line from the TTY device
                line = ser.readline().decode('utf-8').strip()
                if line:
                    print(f"Received: {line}")
    except serial.SerialException as e:
        print(f"Error: {e}")
    except KeyboardInterrupt:
        print("\nExiting...")
    except Exception as e:
        print(f"Unexpected error: {e}")

if __name__ == "__main__":
    tty_device = '/dev/tty.usbserial-10'
    baud_rate = 9600  # Adjust baud rate as needed

    read_from_tty(tty_device, baud_rate)
