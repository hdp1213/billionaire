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
        self.cards = []

    async def _send_from_queue(self):
        """Send commands to the server as they are enqueued

        A command has to have the following items:
            type:       ASK/CANCEL/BILLIONAIRE
            bot_id:     the unique identifier given by the server
            cards:      the cards put forth to trade
        """
        await self._on_join.wait()
        print("Ready to send from command queue")

        while True:
            command = await self.queue.get()
            data = command.to_json()
            self.transport.write(data.encode('utf-8'))
            print(f'SENT {command!r}')

    def connection_made(self, transport):
        """Send the command to the server upon connection"""
        self.transport = transport
        print("Connected to server")

    async def send_command(self, command):
        """Feed a command to the sender coroutine"""
        await self._on_start.wait()
        await self.queue.put(command)

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
            type:           JOIN/START/RECEIVE/CHECK/FINISH
            bot_id:         the unique identifier given by the server
            cards:          the cards received
        """
        try:
            self.received_cmds = CommandList.from_data(data)
        except UnicodeDecodeError as e:
            print('Received invalid unicode')
            return
        except json.decoder.JSONDecodeError as e:
            print('Received invalid JSON')
            return

        print(f'RECEIVED {self.received_cmds!r}')

        if Command.JOIN in self.received_cmds:
            self._on_join.set()

        if Command.START in self.received_cmds:
            print('Starting game...')
            self._on_start.set()

        if Command.FINISH in self.received_cmds:
            print('Stopping game...')
            self._on_start.clear()

    def connection_lost(self, exc):
        """Handle lost connections"""
        print('The server closed the connection')
        print('Stop the event loop')
        self.loop.stop()

    async def issue_command(self):
        """Decide on a command to run"""
        return Command(Command.ASK, cards=[1, 2, 3])


class Command():
    """docstring for Command"""
    JOIN = 'JOIN'
    START = 'START'
    RECEIVE = 'RECEIVE'
    CHECK = 'CHECK'
    FINISH = 'FINISH'
    ASK = 'ASK'
    CANCEL = 'CANCEL'
    BILLIONAIRE = 'BILLIONAIRE'

    valid_commands = {JOIN,
                      START,
                      RECEIVE,
                      CHECK,
                      FINISH,
                      ASK,
                      CANCEL,
                      BILLIONAIRE}

    def __init__(self, command, **attrs):
        if command not in self.valid_commands:
            raise ValueError(f'{command} not a valid command')

        self.command = command
        self._attrs = attrs

    def __eq__(self, other):
        return self.command == other

    def __repr__(self):
        return f'<Command.{self.command}, attrs={self._attrs!r}>'

    def __getattr__(self, name):
        return self._attrs.get(name)

    @classmethod
    def from_dict(cls, data):
        command = data['command']
        attrs = {key: val for key, val in data.items()
                 if key != 'command'}
        return cls(command, **attrs)

    def to_json(self):
        return json.dumps({'command': self.command, **self._attrs},
                          separators=(',', ':'))


class CommandList():
    """docstring for CommandList"""
    def __init__(self, *command_objs):
        self._cmds = {cmd_obj.command: cmd_obj for cmd_obj in command_objs}

    def __contains__(self, cmd_str):
        return cmd_str in self._cmds

    def __getitem__(self, cmd_str):
        return self._cmds.get(cmd_str)

    def __repr__(self):
        return f'<CommandList {list(self._cmds.values())!r}>'

    @classmethod
    def from_data(cls, data):
        # Will throw either UnicodeDecodeError or json.decoder.JSONDecodeError
        commands = json.loads(data.decode())
        command_objs = [Command.from_dict(cmd)
                        for cmd in commands['commands']]
        return cls(*command_objs)

    def to_json(self):
        return json.dumps({'commands':
                           [{'command': cmd.command, **cmd._attrs}
                            for cmd in self._cmds.values()]},
                          separators=(',', ':'))


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
