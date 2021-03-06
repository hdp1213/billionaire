from collections import Counter
import enum
import gi
gi.require_version('Gtk', '3.0')

from gi.repository import Gtk

from card import CardID, CardData


class CommodityField(enum.IntEnum):
    COMMODITY = 0
    AMOUNT = 1
    POINTS = 2

    def __str__(self):
        return f'{self.name.replace("_", " ").title()}'


class CommodityData(Gtk.ListStore, CardData):
    """Interface that stores commodities in a hand"""
    POINTS = 10

    def __init__(self):
        Gtk.ListStore.__init__(self, str, int, int)
        CardData.__init__(self)
        self._iters = {}

    def on_add_new_cards(self, card_id, add_amt):
        card_row = [str(card_id), add_amt, CommodityData.POINTS]
        self._iters[card_id] = self.append(card_row)

    def on_add_present_cards(self, card_id, add_amt):
        card_amt = self.get_amount(card_id)
        self.set_value(self._iters[card_id], CommodityField.AMOUNT, card_amt)

    def on_remove_cards(self, card_id):
        self.remove(self._iters[card_id])
        del self._iters[card_id]

    def on_take_cards(self, card_id, take_amt):
        card_amt = self.get_amount(card_id)
        self.set_value(self._iters[card_id], CommodityField.AMOUNT, card_amt)

    def clear_all(self):
        self.clear_cards()

        self.clear()
        self._iters = {}

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


class CommodityTable(Gtk.TreeView):
    """Object used to display CommodityData as a sortable table"""
    def __init__(self, comm_data):
        Gtk.TreeView.__init__(self, comm_data)

        self.column_names = [str(field) for field in CommodityField]
        self.columns = [None] * len(self.column_names)

        def init_column(col, field, cell_renderer=Gtk.CellRendererText):
            cell = cell_renderer()

            col.pack_start(cell, True)
            col.add_attribute(cell, 'text', field)
            col.set_sort_column_id(field)

        for field in CommodityField:
            self.columns[field] = Gtk.TreeViewColumn(self.column_names[field])
            init_column(self.columns[field], field)
            self.append_column(self.columns[field])

    def get_selected_card(self):
        selection = self.get_selection()
        comm_data, card_iter = selection.get_selected()
        return comm_data.get_card_id(card_iter)
