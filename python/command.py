import json


class Command():
    """docstring for Command"""
    JOIN = 'JOIN'
    START = 'START'
    SUCCESSFUL_TRADE = 'SUCCESSFUL_TRADE'
    CANCELLED_OFFER = 'CANCELLED_OFFER'
    BOOK_EVENT = 'BOOK_EVENT'
    BILLIONAIRE = 'BILLIONAIRE'
    END_ROUND = 'END_ROUND'
    END_GAME = 'END_GAME'
    ERROR = 'ERROR'
    NEW_OFFER = 'NEW_OFFER'
    CANCEL_OFFER = 'CANCEL_OFFER'

    valid_commands = {JOIN,
                      START,
                      SUCCESSFUL_TRADE,
                      CANCELLED_OFFER,
                      BOOK_EVENT,
                      BILLIONAIRE,
                      END_ROUND,
                      END_GAME,
                      ERROR,
                      NEW_OFFER,
                      CANCEL_OFFER}

    def __init__(self, command, **attrs):
        if command not in self.valid_commands:
            raise ValueError(f'{command!r} not a valid command')

        self.command = command
        self._attrs = attrs

    def __eq__(self, other):
        return self.command == other

    def __repr__(self):
        return f'<Command.{self.command}: attrs={self._attrs!r}>'

    def __getattr__(self, name):
        return self._attrs.get(name)

    @classmethod
    def from_dict(cls, data):
        command = data['command']
        attrs = {key: val for key, val in data.items()
                 if key != 'command'}
        return cls(command, **attrs)

    def to_dict(self):
        return {'command': self.command, **self._attrs}

    def to_json(self):
        return json.dumps(self.to_dict(), separators=(',', ':'))


class CommandList():
    """docstring for CommandList"""
    def __init__(self, *command_objs):
        self._cmds = {cmd_obj.command: cmd_obj for cmd_obj in command_objs}

    def __contains__(self, cmd_str):
        return cmd_str in self._cmds

    def __getitem__(self, cmd_str):
        return self._cmds.get(cmd_str)

    def __repr__(self):
        return f'<CommandList: {list(self._cmds.values())!r}>'

    @classmethod
    def from_bytes(cls, data):
        # Will throw either UnicodeDecodeError or json.decoder.JSONDecodeError
        commands = json.loads(data.decode())
        command_objs = [Command.from_dict(cmd)
                        for cmd in commands['commands']]
        return cls(*command_objs)

    def to_dict(self):
        return {'commands': [cmd.to_dict() for cmd in self._cmds.values()]}

    def to_json(self):
        return json.dumps(self.to_dict(), separators=(',', ':'))
