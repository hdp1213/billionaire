from collections import Counter
import enum
import json


class CardType(enum.Enum):
    COMMODITY = 0
    BILLIONAIRE = 1
    TAXMAN = 2


class Commodity(enum.Enum):
    NONE = -1
    DIAMONDS = 0
    GOLD = 1
    OIL = 2
    PROPERTY = 3
    MINING = 4
    SHIPPING = 5
    BANKING = 6
    SPORT = 7


class Card():
    """docstring for Card"""
    def __init__(self, card_type, commodity_type):
        self.card_type = CardType(card_type)
        self.commodity = Commodity(commodity_type)

    def __repr__(self):
        return f'<Card: {self.card_type.name}, {self.commodity.name}>'

    def __hash__(self):
        return hash((self.card_type, self.commodity))

    def __eq__(self, other):
        return ((self.card_type == other.card_type) and
                (self.commodity == other.commodity))

    @classmethod
    def from_dict(cls, data):
        return cls(data['type'], data['commodity'])

    @classmethod
    def from_id(cls, card_id):
        """Return a card given a card_id

        card_id is an identifying enumeration for a card. It is either
        the commodity type (excluding NONE), or the card type (either
        BILLIONAIRE or TAXMAN) (excluding COMMODITY).

        card_id is the minimal amount of information needed to uniquely
        identify a card.
        """
        if isinstance(card_id, CardType) and card_id != CardType.COMMODITY:
            return Card(card_id, Commodity.NONE)

        elif isinstance(card_id, Commodity) and card_id != Commodity.NONE:
            return Card(CardType.COMMODITY, card_id)

        else:
            return None

    def to_dict(self):
        return {'type': self.card_type.value,
                'commodity': self.commodity.value}

    def to_json(self):
        return json.dumps(self.to_dict(), separators=(',', ':'))

    def to_id(self):
        if self.card_type == CardType.COMMODITY:
            return self.commodity

        elif self.commodity == Commodity.NONE:
            return self.card_type

        else:
            return None


class Cards():
    """Container class for Card objects"""
    def __init__(self, *cards):
        self._cards = Counter(cards)

    def __repr__(self):
        return f'<Cards {list(self._cards.elements())!r}>'

    def __len__(self):
        return len(self._cards)

    def __contains__(self, card):
        return card in self._cards

    def amount(self, card_id):
        """Return the number of cards matching card_id"""
        return self._cards.get(Card.from_id(card_id), 0)

    def take_card(self, card_id, take_amt=1):
        card_amt = self.amount(card_id)
        amt_taken = take_amt if take_amt < card_amt else card_amt

        card_dict = {Card.from_id(card_id): amt_taken}
        self._cards.subtract(card_dict)
        self._clean_zeros()

        return Cards.from_dict(card_dict)

    def most_common(self, n=None):
        """Return most common card IDs"""
        return [(card.to_id(), amt)
                for card, amt in self._cards.most_common(n)]

    @classmethod
    def from_json(cls, data):
        """Return a Cards object from a JSON array of Card objects"""
        return cls(*[Card.from_dict(c) for c in data])

    @classmethod
    def from_dict(cls, data):
        """Return a Cards object from a dictionary of (Card, amt) pairs

        Uses a hacky way of getting around the __init__() issue.
        """
        new_cards = cls()
        new_cards._cards = Counter(data)
        return new_cards

    def to_dict(self):
        return {'cards': [card.to_dict() for card in self._cards.elements()]}

    def to_json(self):
        return json.dumps(self.to_dict(), separators=(',', ':'))

    def _clean_zeros(self):
        elems = list(self._cards.keys())

        for card in elems:
            if self._cards[card] < 1:
                del self._cards[card]
