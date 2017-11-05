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

        # asynchronous methods
        asyncio.ensure_future(self._send_from_queue())
        asyncio.ensure_future(self.join_game())
        asyncio.ensure_future(self.start_game())

        # asynchronous events
        self._on_join = asyncio.Event()
        self._on_start = asyncio.Event()

        # Billionaire bot variables
        self.received_cmds = None
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

        join_cmd = self.received_cmds[Command.JOIN]

        self.bot_id = join_cmd.bot_id
        print(f'Joining game with ID {self.bot_id!r}')

    async def start_game(self):
        await self._on_start.wait()

        start_cmd = self.received_cmds[Command.START]
        self.cards = start_cmd.hand
        print(f'Received following hand: {self.cards!r}')

    def data_received(self, data):
        """Receive commands from the server

        The return command consists of two fields:
            type:           JOIN/START/GET
            bot_id:         the unique identifier given by the server
            cards:          the cards received
        """
        try:
            self.received_cmds = CommandList(data)
        except UnicodeDecodeError as e:
            print('Received invalid unicode')
            return
        except json.decoder.JSONDecodeError as e:
            print('Received invalid JSON')
            return

        print(f'Received: {self.received_cmds!r}')

        if Command.JOIN in self.received_cmds:
            self._on_join.set()

        if Command.START in self.received_cmds:
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
        self.command = data['command']
        self._attrs = {key: val for key, val in data.items()
                       if key != 'command'}

    def __eq__(self, other):
        return self.command == other

    def __repr__(self):
        return '<Command.{}, attrs={}>'.format(self.command, repr(self._attrs))

    def __getattr__(self, name):
        return self._attrs.get(name)


class CommandList():
    """docstring for CommandList"""
    def __init__(self, data):
        # Will throw either UnicodeDecodeError or json.decoder.JSONDecodeError
        commands = json.loads(data.decode())
        command_objs = [Command(entry) for entry in commands['commands']]
        self._cmds = {entry.command: entry for entry in command_objs}

    def __contains__(self, cmd_str):
        return cmd_str in self._cmds

    def __getitem__(self, cmd_str):
        return self._cmds.get(cmd_str)

    def __repr__(self):
        return repr(list(self._cmds.values()))


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
