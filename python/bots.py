from base import BaseBillionaireBot

from card import Card, CardType, Commodity
from command import Command, CommandList


class DumbBot(BaseBillionaireBot):
    async def issue_command(self):
        try:
            card_id, amt = self.hand.most_common(1)[0]
        except IndexError:
            card_id, amt = None, 0

        cards = self.hand.take_card(card_id, amt)
        ask = Command(Command.ASK, cards=cards.to_list())
        return ask
