#!/usr/bin/env python

import abc
import asyncio
from json.decoder import JSONDecodeError
import signal

from command import Command, CommandList
from card import Card

TCP_IP = '127.0.0.1'
TCP_PORT = 5555
ADDR = (TCP_IP, TCP_PORT)


class BaseBillionaireProtocol(asyncio.Protocol, metaclass=abc.ABCMeta):
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
        self.cards = [Card.from_dict(c) for c in start_cmd.hand]
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
        except JSONDecodeError as e:
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

        # Cancel all current tasks
        for event in asyncio.Task.all_tasks(self.loop):
            event.cancel()

        # Stop the current loop
        self.loop.stop()

    @abc.abstractmethod
    async def issue_command(self):
        """Decide on a command to run"""
        return NotImplemented


class DumbBot(BaseBillionaireProtocol):
    async def issue_command(self):
        return Command(Command.ASK, cards=[1, 2, 3])


class BotDriver():
    """docstring for BotDriver"""
    def __init__(self, address, protocol_cls, loop=None):
        self.address = address
        self.loop = loop if loop else asyncio.get_event_loop()
        self.protocol = protocol_cls(loop)

        asyncio.ensure_future(self.command_loop())

    def run(self):
        self.loop.add_signal_handler(signal.SIGTERM, self.sig_handle)
        self.loop.add_signal_handler(signal.SIGINT, self.sig_handle)

        coro = self.loop.create_connection(lambda: self.protocol,
                                           *self.address)
        try:
            self.loop.run_until_complete(coro)
            self.loop.run_forever()

        except ConnectionRefusedError:
            print('Connection to {}:{} failed'.format(*self.address))

        self.end_gracefully()

    def end_gracefully(self):
        for event in asyncio.Task.all_tasks(self.loop):
            event.cancel()

        self.loop.stop()

    def sig_handle(self):
        print('Closing connection')
        self.end_gracefully()

    async def command_loop(self):
        while True:
            command = await self.protocol.issue_command()
            await self.protocol.send_command(command)
            await asyncio.sleep(1)


if __name__ == '__main__':
    driver = BotDriver(ADDR, DumbBot)
    driver.run()
