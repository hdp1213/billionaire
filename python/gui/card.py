import enum


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
