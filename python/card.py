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

    @classmethod
    def from_dict(cls, data):
        return cls(data['type'], data['commodity'])

    def to_dict(self):
        return {'type': self.card_type,
                'commodity': self.commodity}

    def to_json(self):
        return json.dumps(self.to_dict(), separators=(',', ':'))
