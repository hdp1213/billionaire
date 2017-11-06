from base import BaseBillionaireBot

from card import Card
from command import Command, CommandList


class DumbBot(BaseBillionaireBot):
    async def issue_command(self):
        ask = Command(Command.ASK, cards=[1])
        return ask
