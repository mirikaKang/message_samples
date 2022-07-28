import sys
sys.path.append('/messaging_system')

import logging
from messaging_system import messaging_system as ms

logging.basicConfig(stream=sys.stderr, level=logging.INFO)

client = ms.messaging_client('echo_client', 'echo_network', 231, 67)
client.start('127.0.0.1', 9876)

echo_packet = ms.container()
echo_packet.create('', '', 'echo_server', '', 'echo_test')
client.send_packet(echo_packet)

message = client.recv_packet()
if message.message_type != "echo_test":
    client.stop()
    exit()

logging.info('received echo message from server')

client.stop()