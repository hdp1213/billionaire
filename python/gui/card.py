from collections import Counter
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
        return f'{self.name.replace("_", " ").title()}'

    def __repr__(self):
        return f'{type(self).__name__}.{self.name}'


class CardData():
    """Abstract class containing card amount information"""
    def __init__(self):
        self._cards = Counter()
        self._values = {}

    @classmethod
    def from_dict(cls, amt_data, val_data):
        card_data = cls()
        card_data._cards = Counter(amt_data)
        card_data._values = val_data
        return card_data

    def __len__(self):
        """Return the total number of cards contained in CardData"""
        return sum(self._cards.values())

    def get_amount(self, card_id):
        """Return the number of cards matching card_id"""
        return self._cards[card_id]

    def add_cards(self, card_id, add_amt):
        """Add a number of cards to the CardData"""
        assert add_amt > 0

        if not self.is_valid_add(card_id, add_amt):
            return

        if card_id not in self._cards:
            self._cards.update({card_id: add_amt})
            self.on_add_new_cards(card_id, add_amt)
        else:
            self._cards.update({card_id: add_amt})
            self.on_add_present_cards(card_id, add_amt)

    def take_cards(self, card_id, take_amt):
        """Take a number of cards from the CardData object

        Amount taken cannot exceed amount of cards present."""
        if card_id not in self._cards:
            return

        assert take_amt > 0

        if not self.is_valid_take(card_id, take_amt):
            return

        card_amt = self.get_amount(card_id)

        if take_amt > card_amt:
            raise ValueError
        elif take_amt == card_amt:
            del self._cards[card_id]
            self.on_remove_cards(card_id)
        else:
            self._cards.subtract({card_id: take_amt})
            self.on_take_cards(card_id, take_amt)

    def add_card(self, card_id):
        self.add_cards(card_id, 1)

    def take_card(self, card_id):
        self.take_cards(card_id, 1)

    def clear_cards(self):
        self._cards.clear()

    def on_add_new_cards(self, card_id, add_amt):
        """Called when a new card_id not in the object is added"""
        raise NotImplementedError

    def on_add_present_cards(self, card_id, add_amt):
        """Called when a card_id already present is added"""
        raise NotImplementedError

    def on_remove_cards(self, card_id):
        """Called when a card_id is completely removed"""
        raise NotImplementedError

    def on_take_cards(self, card_id, take_amt):
        """Called when a number of card_id is taken

        The amount taken will always be less than the amount contained
        in the object. If the amount taken is the same, then
        on_remove_cards() is called instead.
        """
        raise NotImplementedError

    def is_valid_add(self, card_id, add_amt):
        """An optional validation check for card adding"""
        return True

    def is_valid_take(self, card_id, take_amt):
        """An optional validation check for card taking"""
        return True
