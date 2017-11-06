import abc
import asyncio
from functools import wraps
from json.decoder import JSONDecodeError
import signal

from card import Cards
from command import Command, CommandList


class BotMeta(abc.ABCMeta):
    """Metaclass to decorate issue_command with"""
    def __new__(cls, *args):
        """Ensure child class conformality with server

        New instances will have issue_command() wrapped such that the
        implementor does not need to worry about wrapping a command
        themselves. All issued commands will be automatically wrapped in
        a CommandList before being sent.
        """
        new_class = super(BotMeta, cls).__new__(cls, *args)
        new_class.issue_command = cls.command_wrap(new_class.issue_command)
        return new_class

    @classmethod
    def command_wrap(cls, issue_command):
        """Decorator to force issue_command() to return a CommandList"""
        @wraps(issue_command)
        async def wrapped_issue_command(self):
            res = await issue_command(self)

            if isinstance(res, Command):
                return CommandList(res)

            elif isinstance(res, (list, tuple)):
                return CommandList(*res)

            elif isinstance(res, CommandList):
                return res

            # TODO: stop execution nicely if return value is
            # incompatible with server
            else:
                name = type(self).__name__
                print(f'Return value from {name}.issue_command() '
                      'is incompatible with server. '
                      'Expected Command, tuple or list of Commands, '
                      'or a CommandList.')
                self.loop.stop()

        return wrapped_issue_command


class BaseBillionaireBot(asyncio.Protocol, metaclass=BotMeta):
    """Abstract Bot class that each bot must inherit from

    This class handles all client logic including how to send and
    receive commands, and how to react to certain commands.
    """
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
        self.received_cmds = CommandList()
        self.id = ''
        self.hand = Cards()

    async def _send_from_queue(self):
        """Send commands to the server as they are enqueued

        A sent command must have the following field:
            command:    ASK/CANCEL/BILLIONAIRE

        Optionally, a sent command may also contain:
            bot_id:     the unique identifier given by the server
            cards:      the cards put forth to trade
        """
        await self._on_join.wait()
        print('Ready to send from command queue')

        while True:
            command = await self.queue.get()
            data = command.to_json().encode('utf-8')
            self.transport.write(data)
            print(f'SENT {command!r}')

    async def send_command(self, command):
        """Enqueue a command for later sending by _send_from_queue()"""
        await self._on_start.wait()
        await self.queue.put(command)

    def connection_made(self, transport):
        """Add server transport upon connection"""
        self.transport = transport
        print('Connected to server')

    def data_received(self, data):
        """Receive commands from the server

        A received command must have the following field:
            command:        JOIN/START/RECEIVE/BOOK_STATE/CHECK/FINISH

        Optionally, a received command may also contain:
            bot_id:         the unique identifier given by the server
            hand:           the starting hand
            cards:          the cards received from a trade
        """
        try:
            self.received_cmds = CommandList.from_bytes(data)
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

        if Command.RECEIVE in self.received_cmds:
            pass

        if Command.BOOK_STATE in self.received_cmds:
            pass

        if Command.CHECK in self.received_cmds:
            pass

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

    async def join_game(self):
        await self._on_join.wait()

        join_cmd = self.received_cmds[Command.JOIN]

        self.bot_id = join_cmd.bot_id
        print(f'Joining game with ID {self.bot_id!r}')

    async def start_game(self):
        await self._on_start.wait()

        start_cmd = self.received_cmds[Command.START]

        self.hand = Cards.from_json(start_cmd.hand)
        print(f'Received following hand: {self.hand!r}')

    @abc.abstractmethod
    async def issue_command(self):
        """Decide on a command to run

        This command must be implemented by each bot inheriting from the
        base abstract class.
        """
        return NotImplemented


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

        name = type(self.protocol).__name__
        print(f'Running {name!r} bot..')

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
