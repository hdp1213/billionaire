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

    def add_offer(self, offer_amt, owner_id):
        # If the offer already exists, the offer is about to be traded
        # out anyway, so ignore
        if offer_amt in self._offers:
            return

        offer_row = [offer_amt, owner_id]
        self._iters[offer_amt] = self.append(offer_row)
        self._offers[offer_amt] = owner_id

    def remove_offer(self, offer_amt):
        self.remove(self._iters[offer_amt])
        del self._iters[offer_amt]
        del self._offers[offer_amt]

    def owner_id_of(self, offer_amt):
        return self._offers[offer_amt]

    def clear_all(self):
        self.clear()
        self._iters = {}
        self._offers = {}

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
        Gtk.TreeView.__init__(self, offer_data)

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
    @GObject.Signal
    def quick_trade(self, trade_amt: int):
        pass

    @GObject.Signal
    def cancel_offer(self, offer_amt: int):
        pass

    def __init__(self):
        Gtk.Frame.__init__(self, label='Offers')

        self.data = OfferData()
        self.table = OfferTable(self.data)

        self.is_comm_selected = False
        self.is_offer_selected = False

        self.table.connect('cursor-changed', self.on_selection)
        self.table.connect('row-activated', self.on_quick_trade)

        self.match_btn = Gtk.Button(label='Match')
        self.cancel_btn = Gtk.Button(label='Cancel')

        self.match_btn.connect('clicked', self.on_match)
        self.cancel_btn.connect('clicked', self.on_cancel)

        self.match_btn.set_sensitive(False)
        self.cancel_btn.set_sensitive(False)

        self.grid = Gtk.Grid()

        self.grid.attach(self.table, 1, 1, 1, 2)
        self.grid.attach(self.match_btn, 2, 1, 1, 1)
        self.grid.attach(self.cancel_btn, 2, 2, 1, 1)

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

    def update_ui(self):
        self.match_btn.set_sensitive(self.is_comm_selected and
                                     self.is_offer_selected)

        self.cancel_btn.set_sensitive(self.is_offer_selected)

    def clear_all(self):
        self.is_comm_selected = False
        self.is_offer_selected = False

        self.data.clear_all()
        self.update_ui()

    def on_selection(self, widget):
        self.is_offer_selected = True
        self.update_ui()

    def on_quick_trade(self, widget, path, column):
        """Emit a quick-trade signal containing offer amount"""
        offer_iter = self.data.get_iter(path)
        trade_amt = self.data.get_offer_amount(offer_iter)

        self.emit('quick-trade', trade_amt)

    def on_match(self, button):
        """Emit a quick-trade signal containing offer amount"""
        trade_amt = self.table.get_selected_offer()

        self.emit('quick-trade', trade_amt)

    def on_cancel(self, button):
        offer_amt = self.table.get_selected_offer()

        self.emit('cancel-offer', offer_amt)
