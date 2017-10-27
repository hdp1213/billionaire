#!/usr/bin/env python

import asyncio
import json

TCP_IP = '127.0.0.1'
TCP_PORT = 5555
ADDR = (TCP_IP, TCP_PORT)

class JSONProtocol(asyncio.Protocol):
    def __init__(self, loop=None):
        self.transport = None
        self.loop = loop if loop else asyncio.get_event_loop()

        # asynchronous queue
        self.queue = asyncio.Queue()
        asyncio.ensure_future(self._send_from_queue())
        asyncio.ensure_future(self.join_game())

        # asynchronous events
        self._on_join = asyncio.Event()
        self._on_start = asyncio.Event()

        # Billionaire bot variables
        self.received = None
        self.id = None
        self._state = None
        self.cards = []

    async def _send_from_queue(self):
        """Send commands to the server as they are enqueued

        A command has to have the following items:
            type:       ASK/CANCEL/BILLIONAIRE
            bot_id:     the unique identifier given by the server
            cards:      the cards put forth to trade
        """
        await self._on_join.wait()
        print("Ready to send from queue")

        while True:
            data = await self.queue.get()
            self.transport.write(str(data).encode('utf-8'))
            print(f'Sent: {data!r}')

    def connection_made(self, transport):
        """Send the command to the server upon connection"""
        self.transport = transport
        print("Connected to server")

    async def send_command(self, data):
        """Feed a command to the sender coroutine"""
        await self._on_start.wait()
        await self.queue.put(data)

    async def join_game(self):
        await self._on_join.wait()

        self.bot_id = self.received.bot_id
        self.cards = self.received.cards
        print(f'Joining game with ID {self.bot_id!r}')
        print(f'Have following hand: {self.cards!r}')

    def data_received(self, data):
        """Receive commands from the server

        The return command consists of two fields:
            type:           JOIN/START/GET
            bot_id:         the unique identifier given by the server
            cards:          the cards received
        """
        try:
            self.received = Command(data)
        except UnicodeDecodeError as e:
            print('Received invalid unicode')
            return
        except json.decoder.JSONDecodeError as e:
            print('Received invalid JSON')
            return

        print(f'Received: {self.received!r}')

        if self.received == Command.JOIN:
            self._on_join.set()

        elif self.received == Command.START:
            print('Starting game...')
            self._on_start.set()

    def connection_lost(self, exc):
        """Handle lost connections"""
        print('The server closed the connection')
        print('Stop the event loop')
        self.loop.stop()

    async def issue_command(self):
        """Decide on a command to run"""
        command = json.dumps({'command': 'ASK', 'cards': [1,2,3]},
                             separators=(',', ':'))
        return command


class Command():
    """docstring for Command"""
    JOIN        = 'JOIN'
    START       = 'START'
    RECEIVE     = 'RECEIVE'
    CHECK       = 'CHECK'
    FINISH      = 'FINISH'
    ASK         = 'ASK'
    CANCEL      = 'CANCEL'
    BILLIONAIRE = 'BILLIONAIRE'

    valid_commands = {JOIN,
                      START,
                      RECEIVE,
                      CHECK,
                      FINISH,
                      ASK,
                      CANCEL,
                      BILLIONAIRE}

    def __init__(self, data):
        # Will throw either UnicodeDecodeError or json.decoder.JSONDecodeError
        self._data = json.loads(data.decode())

    def __eq__(self, other):
        return self.command == other

    def __repr__(self):
        return repr(self._data)

    def __getattr__(self, name):
        return self._data.get(name)


class BotDriver():
    """docstring for BotDriver"""
    def __init__(self, address, loop=None):
        self.address = address
        self.loop = loop if loop else asyncio.get_event_loop()
        self.protocol = JSONProtocol(loop)

        asyncio.ensure_future(self.command_loop())

    def run(self):
        coro = self.loop.create_connection(lambda: self.protocol,
                                             *self.address)
        self.loop.run_until_complete(coro)

        try:
            self.loop.run_forever()
        except KeyboardInterrupt:
            print('Closing connection')
            self.loop.stop()

    async def command_loop(self):
        while True:
            command = await self.protocol.issue_command()
            await self.protocol.send_command(command)
            await asyncio.sleep(1)

if __name__ == '__main__':
    driver = BotDriver(ADDR)
    driver.run()
