import sys
import telnetlib

tn = telnetlib.Telnet('localhost', 6379)

def decode_escape_sequences(string):
    return string.encode('utf-8').decode('unicode_escape').encode('latin-1')

while True:
    # Example command following RESP format that you can feed through command line
    # *3\r\n$3\r\nset\r\n$4\r\nname\r\n$5\r\ndenis\r\n
    # *3\r\n$3\r\ndel\r\n$4\r\nname\r\n$5\r\ndenis\r\n
    # *3\r\n$3\r\nset\r\n$3\r\nage\r\n$1\r\n9\r\n


    command = input("Enter Redis command: ")
    byte_command = decode_escape_sequences(command)
    print(byte_command)

    tn.write(byte_command)

    response = tn.read_some()
    print(response.decode('ascii'))
