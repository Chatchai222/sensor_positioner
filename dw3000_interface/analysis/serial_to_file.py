import serial
import argparse 


def append_to_file(filename, in_line):
    with open(filename, "a") as file:
        file.write(in_line)


def main():
    print("Hello from range analysis")
    parser = argparse.ArgumentParser(prog="serial to text file")
    parser.add_argument("filename")
    parser.add_argument("serial_port")
    parser.add_argument("baud_rate")
    
    print("The argument passed is: ")
    args = parser.parse_args()

    filename = args.filename
    serial_port = args.serial_port
    baud_rate = int(args.baud_rate)
    print("filename: ", filename)
    print("serial_port: ", serial_port)
    print("baud_rate: ", baud_rate)
    
    print("about to append to file: ")
    with serial.Serial(serial_port, baud_rate) as ser:
        while True:
            line = ser.readline()
            line = line.decode()
            line = line.strip('\r')
            append_to_file(filename, line)
            print(line, end='')


if __name__ == "__main__":
    main()