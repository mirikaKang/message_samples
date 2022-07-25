import sys
from messaging_system.python import *

server_address = ('127.0.0.1', 9876)
client = messaging_client('echo_network', 231, 67)
client.start(server_address)
client.send_packet('@header={'
                    '[1,unknown];'
                    '[2,127.0.0.1:9876];'
                    '[3,echo_client];'
                    '[4,];'
                    '[5,request_connection];'
                    '[6,1.0.0.0];'
                '};'
                '@data={'
                    '[connection_key,d,echo_network];'
                    '[auto_echo,1,false];'
                    '[auto_echo_interval_seconds,3,1];'
                    '[session_type,2,1];'
                    '[bridge_mode,1,false];'
                    '[snipping_targets,e,0];'
                '};')

print(client.recv_packet())

client.stop()