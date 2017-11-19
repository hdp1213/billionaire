from collections import Counter
import enum
import json


class CardID(enum.Enum):
    # Invalid card
    INVALID = -1
    # Commodities
    DIAMONDS = 0
    GOLD = 1
    OIL = 2
    PROPERTY = 3
    MINING = 4
    SHIPPING = 5
    BANKING = 6
    SPORT = 7
    # Special cards
    BILLIONAIRE = 8
    TAX_COLLECTOR = 9

    def __str__(self):
        return f'{type(self).__name__}.{self.name}'


class CardLocation():
    """Container class for Card objects"""
    def __init__(self, cards=None):
        self._cards = Counter() if cards is None else Counter(cards)

    def __repr__(self):
        return f'<CardLocation: {{{self.to_pretty_list()}}}>'

    def __len__(self):
        return len(self._cards)

    def __contains__(self, card):
        return card in self._cards

    def amount(self, card_id):
        """Return the number of cards matching card_id"""
        return self._cards[card_id]

    def take_card(self, card_id, take_amt=1):
        card_amt = self.amount(card_id)
        amt_taken = take_amt if take_amt < card_amt else card_amt

        card_dict = {card_id: amt_taken}
        self._cards.subtract(card_dict)
        self._clean_zeros()

        return CardLocation(card_dict)

    def most_common(self, n=None):
        """Return most common card IDs"""
        return [(card_id, card_amt)
                for card_id, card_amt in self._cards.most_common(n)]

    @classmethod
    def from_json(cls, data):
        """Return a CardLocation object from a JSON CardLocation object"""
        data_dict = {CardID(card.get('id', CardID.INVALID)): card.get('amt', 0)
                     for card in data}
        return cls(data_dict)

    def to_json(self):
        return json.dumps(self.to_list(), separators=(',', ':'))

    def to_list(self):
        return [{"id": card_id.value, "amt": card_amt}
                for card_id, card_amt in self._cards.items()]

    def to_pretty_list(self):
        return ', '.join([f'{card_id}: {card_amt}'
                          for card_id, card_amt in self._cards.items()])

    def _clean_zeros(self):
        elems = list(self._cards.keys())

        for card in elems:
            if self._cards[card] < 1:
                del self._cards[card]
