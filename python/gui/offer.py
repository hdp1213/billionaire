import enum
import gi
gi.require_version('Gtk', '3.0')

from gi.repository import GLib, Gtk, GObject

from card import CardID


class OfferField(enum.IntEnum):
    AMOUNT = 0
    OWNER = 1

    def __str__(self):
        return f'{self.name.replace("_", " ").title()}'


class OfferData(Gtk.ListStore):
    """Object storing current offers"""
    def __init__(self):
        Gtk.ListStore.__init__(self, int, str)

        self._iters = {}
        self._offers = {}

        self.add_offer(3, 'Rachel')
        self.add_offer(5, 'Johnny')

    def add_offer(self, offer_amt, owner_id):
        # if the offer already exists, something has gone wrong
        if offer_amt in self._offers:
            raise ValueError('NEW_OFFERs cannot overlap in this way')

        offer_row = [offer_amt, owner_id]
        self._iters[offer_amt] = self.append(offer_row)
        self._offers[offer_amt] = owner_id

    def remove_offer(self, offer_amt):
        self.remove(self._iters[offer_amt])
        del self._iters[offer_amt]
        del self._offers[offer_amt]

    def get_offer_amount(self, tree_iter):
        """Return the offer amount corresponding to a Gtk.TreeIter object

        See CommodityData.get_card_id() for an explanation
        """
        if tree_iter is None:
            return None

        inv_data = {val.user_data: key for key, val in self._iters.items()}
        return inv_data[tree_iter.user_data]


class OfferTable(Gtk.TreeView):
    """Object used to display OfferData as a sortable table"""
    def __init__(self, offer_data):
        super(OfferTable, self).__init__(offer_data)

        self.column_names = [str(field) for field in OfferField]
        self.columns = [None] * len(self.column_names)

        def init_column(col, field, cell_renderer=Gtk.CellRendererText):
            cell = cell_renderer()

            col.pack_start(cell, True)
            col.add_attribute(cell, 'text', field)
            col.set_sort_column_id(field)

        for field in OfferField:
            self.columns[field] = Gtk.TreeViewColumn(self.column_names[field])
            init_column(self.columns[field], field)
            self.append_column(self.columns[field])

        self.set_name('offer-table')

    def get_selected_offer(self):
        selection = self.get_selection()
        offer_data, offer_iter = selection.get_selected()
        return offer_data.get_offer_amount(offer_iter)


class OfferDisplay(Gtk.Frame):
    """Display container for an OfferTable object"""
    def __init__(self):
        Gtk.Frame.__init__(self, label='Offers')

        self.offer_data = OfferData()
        self.offer_tab = OfferTable(self.offer_data)

        self.offer_tab.connect('row-activated', self.on_quick_trade)

        self.grid = Gtk.Grid()

        self.grid.attach(self.offer_tab, 1, 1, 1, 1)

        self.grid.set_column_spacing(15)
        self.grid.set_row_spacing(5)

        self.grid.set_margin_start(5)
        self.grid.set_margin_end(5)
        self.grid.set_margin_top(5)
        self.grid.set_margin_bottom(5)

        self.set_margin_start(5)
        self.set_margin_end(5)
        self.set_margin_top(5)
        self.set_margin_bottom(5)

        self.add(self.grid)
        self.set_name('offer-display')

    def on_quick_trade(self, widget, path, column):
        pass
