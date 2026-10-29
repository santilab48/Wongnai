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

class StreamTests(unittest.TestCase):
    def test_large_payload(self):
        listener=socket.socket();listener.bind(('127.0.0.1',0));listener.listen()
        data=bytes(range(256))*256;got=[]
        def receive():
            conn,_=listener.accept()
            with conn:
                while part:=conn.recv(4096):got.append(part)
        t=threading.Thread(target=receive);t.start()
        send('127.0.0.1',listener.getsockname()[1],data,137)
        t.join(5);listener.close();self.assertEqual(b''.join(got),data)
    def test_concurrent_clients(self):
        listener=socket.socket();listener.bind(('127.0.0.1',0));listener.listen(8)
        port=listener.getsockname()[1];got=[]
        def receive():
            for _ in range(4):
                conn,_=listener.accept()
                with conn:got.append(conn.recv(100))
        t=threading.Thread(target=receive);t.start()
        clients=[threading.Thread(target=send,args=('127.0.0.1',port,bytes([i]))) for i in range(4)]
        for c in clients:c.start()
        for c in clients:c.join()
        t.join(5);listener.close();self.assertCountEqual(got,[bytes([i]) for i in range(4)])
    def test_network_failure(self):
        listener=socket.socket();listener.bind(('127.0.0.1',0));port=listener.getsockname()[1];listener.close()
        with self.assertRaises(OSError):send('127.0.0.1',port,b'test')
