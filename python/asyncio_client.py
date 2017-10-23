#!/usr/bin/env python

import asyncio
import json
import random

TCP_IP = '127.0.0.1'
TCP_PORT = 5555
ADDR = (TCP_IP, TCP_PORT)

class SubscriberClientProtocol(asyncio.Protocol):
    def __init__(self, loop):
        random.seed()
        self.transport = None
        self.loop = loop
        self.queue = asyncio.Queue()
        self._ready = asyncio.Event()
        asyncio.ensure_future(self._send_from_queue())

    @asyncio.coroutine
    def _send_from_queue(self):
        """ Send messages to the server as they become available. """
        yield from self._ready.wait()
        print("Ready to send from queue")
        while True:
            data = yield from self.queue.get()
            self.transport.write(str(data).encode('utf-8'))
            print('Message sent: {!r}'.format(data))

    def connection_made(self, transport):
        """ Upon connection send the message to the
        server

        A message has to have the following items:
            type:       subscribe/unsubscribe
            channel:    the name of the channel
        """
        self.transport = transport
        print("Connection made.")
        self._ready.set()

    @asyncio.coroutine
    def send_message(self, data):
        """ Feed a message to the sender coroutine. """
        yield from self.queue.put(data)

    def data_received(self, data):
        """ After sending a message we expect a reply
        back from the server

        The return message consist of three fields:
            type:           subscribe/unsubscribe
            channel:        the name of the channel
            channel_count:  the amount of channels subscribed to
        """
        print('Message received: {!r}'.format(data.decode()))

    def connection_lost(self, exc):
        print('The server closed the connection')
        print('Stop the event loop')
        self.loop.stop()

    @asyncio.coroutine
    def create_message(self):
        message = str(random.randint(0, 10))
        return message


@asyncio.coroutine
def feed_messages(protocol):
    """ An example function that sends the same message repeatedly. """
    while True:
        message = yield from protocol.create_message()
        yield from protocol.send_message(message)
        yield from asyncio.sleep(1)


if __name__ == '__main__':
    loop = asyncio.get_event_loop()
    coro = loop.create_connection(lambda: SubscriberClientProtocol(loop),
                                  *ADDR)

    _, proto = loop.run_until_complete(coro)

    asyncio.ensure_future(feed_messages(proto))

    try:
        loop.run_forever()
    except KeyboardInterrupt:
        print('Closing connection')

    loop.close()
