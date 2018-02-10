from base import BaseBillionaireBot

from card import CardID, CardLocation
from command import Command, CommandList


class DumbBot(BaseBillionaireBot):
    async def issue_command(self):
        try:
            card_id, amt = self.hand.most_common(1)[0]
        except IndexError:
            return None

        cards = self.hand.take_card(card_id, amt)
        # new_offer = self.new_offer(cards)
        new_offer = Command(Command.NEW_OFFER, cards=cards.to_list())
        return new_offer
