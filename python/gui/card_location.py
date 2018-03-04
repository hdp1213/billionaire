from card import CardData, CardID
from utils import JSONInterface


class CardLocation(CardData, JSONInterface):
    """An abstract CardLocation that stores cards

    This class does not interface with any GUI elements, and as such
    could be used as part of a bot.

    All optional actions are ignored, and only the core behaviour of
    CardData is implemented.
    """
    def __init__(self):
        super(CardLocation, self).__init__()

    def __repr__(self):
        return f'<CardLocation: {{{self.pretty_list()}}}>'

    def unique_card_list(self):
        return ', '.join([f'{card_id}' for card_id in self._cards])

    def pretty_list(self):
        return ', '.join([f'{card_id!r}: {card_amt}'
                          for card_id, card_amt in self._cards.items()])

    def to_dict(self):
        return [{"id": card_id.value, "amt": card_amt}
                for card_id, card_amt in self._cards.items()]

    @classmethod
    def from_json(cls, data):
        """Return a CardLocation object from a JSON CardLocation object"""
        data_dict = {CardID(card.get('id', CardID.INVALID)): card.get('amt', 0)
                     for card in data}

        return cls.from_dict(data_dict)

    def on_add_new_cards(self, card_id, add_amt):
        pass

    def on_add_present_cards(self, card_id, add_amt):
        pass

    def on_remove_cards(self, card_id):
        pass

    def on_take_cards(self, card_id, take_amt):
        pass

    def is_valid_add(self, card_id, add_amt):
        return True

    def is_valid_take(self, card_id, take_amt):
        return True
