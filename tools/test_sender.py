import socket
import threading
import unittest
from tcp_sender import payload, send

class SenderTests(unittest.TestCase):
    def test_payloads(self):
        self.assertIn('โต๊ะ'.encode(),payload('thai'))
        self.assertEqual(len(payload('binary')),256)
        self.assertTrue(payload('escpos').startswith(b'\x1b@'))
    def test_chunked_stream(self):
        listener=socket.socket();listener.bind(('127.0.0.1',0));listener.listen()
        got=[]
        def receive():
            conn,_=listener.accept()
            with conn:
                while True:
                    part=conn.recv(3)
                    if not part:break
                    got.append(part)
        thread=threading.Thread(target=receive);thread.start()
        send('127.0.0.1',listener.getsockname()[1],b'abcdef',2)
        thread.join(2);listener.close()
        self.assertEqual(b''.join(got),b'abcdef')
if __name__=='__main__':unittest.main()
