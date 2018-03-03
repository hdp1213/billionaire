from collections import Counter
import enum
import gi
gi.require_version('Gtk', '3.0')

from gi.repository import Gtk

from card import CardID


class HandField(enum.IntEnum):
    COMMODITY = 0
    AMOUNT = 1
    POINTS = 2

    def __str__(self):
        return f'{type(self).__name__}.{self.name}'


class Hand(Gtk.ListStore):
    """Interface that displays a given hand"""
    POINTS = 10

    def __init__(self):
        super(Hand, self).__init__(str, int, int)
        self._cards = Counter()
        self._iters = {}

        self.add_cards(CardID.DIAMONDS, 5)
        self.add_cards(CardID.GOLD, 2)

    def __len__(self):
        return len(self._cards)

    def __contains__(self, card):
        return card in self._cards

    def get_amount(self, card_id):
        """Return the number of cards matching card_id"""
        return self._cards[card_id]

    def add_cards(self, card_id, add_amt):
        """Add a number of cards to the Hand"""
        if card_id not in self:
            card_row = [card_id.name.title(), add_amt, Hand.POINTS]
            self._iters[card_id] = self.append(card_row)
        else:
            new_amt = self.get_amount(card_id) + add_amt
            self.set_value(self._iters[card_id], HandField.AMOUNT, new_amt)

        self._cards.update({card_id: add_amt})

    def take_cards(self, card_id, take_amt):
        """Take a number of cards from the Hand

        Amount taken can exceed amount of cards in hand"""
        if card_id not in self:
            return

        card_amt = self.get_amount(card_id)

        if take_amt > card_amt:
            return
        elif take_amt == card_amt:
            self.remove(self._iters[card_id])
            del self._cards[card_id]
            del self._iters[card_id]
        else:
            new_amt = card_amt - take_amt
            self.set_value(self._iters[card_id], HandField.AMOUNT, new_amt)

            self._cards.subtract({card_id: take_amt})

    def get_card_id(self, tree_iter):
        """Return the CardID corresponding to a Gtk.TreeIter object

        As this method is called with Gtk.TreeIter objects not
        necessarily identical to those in the self._iters dict, the
        corresponding CardID is found by using the user_data field as
        the key, rather than the Gtk.TreeIter object itself.

        This is because different Gtk.TreeIter objects have different
        hashes even if they correspond to the same row. The constant
        across rows should always be the user_data field.
        """
        if tree_iter is None:
            return CardID.INVALID

        inv_data = {val.user_data: key for key, val in self._iters.items()}
        return inv_data[tree_iter.user_data]


class HandTable(Gtk.TreeView):
    """Object used to display the Hand object"""
    def __init__(self, hand):
        super(HandTable, self).__init__(hand)

        self.column_names = [field.name.title() for field in HandField]
        self.columns = [None] * len(self.column_names)

        def link_col(col, field, cell_renderer=Gtk.CellRendererText):
            cell = cell_renderer()

            col.pack_start(cell, True)
            col.add_attribute(cell, 'text', field)
            col.set_sort_column_id(field)

        for field in HandField:
            self.columns[field] = Gtk.TreeViewColumn(self.column_names[field])
            link_col(self.columns[field], field)
            self.append_column(self.columns[field])

    def get_selected_card(self):
        selection = self.get_selection()
        hand, card_iter = selection.get_selected()
        return hand.get_card_id(card_iter)
