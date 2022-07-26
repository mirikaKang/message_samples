import sys
sys.path.append('./messaging_system')

from messaging_system import *

client = messaging_client('echo_client', 'echo_network', 231, 67)
client.start('127.0.0.1', 9876)

echo_packet = container()
echo_packet.create('', '',
                    'echo_server', '', 'echo_test', [])
client.send_packet(echo_packet)
print(client.recv_packet().serialize())        
client.stop()