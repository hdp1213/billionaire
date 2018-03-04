import gi
gi.require_version('Gtk', '3.0')

from gi.repository import Gtk

from card import CardID
from commodity import CommodityData, CommodityTable
from wildcard import Wildcards


class HandDisplay(Gtk.Grid):
    """Object displaying hand information and offer selection"""
    DEFAULT_OFFER_AMT = 0
    MIN_OFFER_AMT = 2

    def __init__(self):
        super(HandDisplay, self).__init__()

        self.selected_comm = CardID.INVALID

        self.comm = CommodityData()
        self.comm_tab = CommodityTable(self.comm)
        self.comm_tab.connect('cursor-changed', self.on_selection)
        self.comm_tab.connect('row-activated', self.on_quick_offer)

        offer_amt = Gtk.Adjustment(step_increment=1)
        self.offer_amt = Gtk.SpinButton(adjustment=offer_amt)
        self.offer_amt.set_numeric(True)
        self.offer_amt.connect('activate', self.on_offer)

        self.wild = Wildcards()
        self.wild.connect_check_buttons(self.on_toggle)

        self.offer_btn = Gtk.Button(label='Offer')
        self.offer_btn.connect('clicked', self.on_offer)
        self.update_offer_ui(valid=False)

        self.attach(self.comm_tab, 1, 1, 1, 3)
        self.attach(self.wild, 2, 1, 1, 2)
        self.attach(self.offer_amt, 2, 3, 1, 1)
        self.attach(self.offer_btn, 3, 3, 1, 1)

        self.set_column_spacing(15)
        self.set_row_spacing(5)

        self.set_margin_start(5)
        self.set_margin_end(5)
        self.set_margin_top(5)
        self.set_margin_bottom(5)
        self.set_name('hand-grid')

    def get_min_offer(self):
        """Return the minimum offer for the given selection

        We note that selecting wildcards influences this. A wildcard can
        only be traded alongside a commodity, so when two wildcards are
        selected, an extra card must be offered and so the minimum
        """
        return (HandDisplay.MIN_OFFER_AMT +
                len(self.wild) // len(Wildcards.SET))

    def get_max_offer(self):
        """Return the maximum offer amount for the given selection"""
        return self.comm.get_amount(self.selected_comm) + len(self.wild)

    def valid_selection(self):
        comm_amt = self.comm.get_amount(self.selected_comm)
        return (self.selected_comm != CardID.INVALID and
                (comm_amt + len(self.wild)) >= HandDisplay.MIN_OFFER_AMT)

    def update_offer_ui(self, valid=None):
        """Updates commodity amount ranges and offer button

        The validation check can be overloaded by setting valid to a
        boolean.
        """
        if valid is None:
            valid = self.valid_selection()

        if valid:
            self.offer_amt.set_range(self.get_min_offer(),
                                     self.get_max_offer())
            self.offer_amt.set_value(self.get_min_offer())
            self.offer_btn.set_sensitive(True)
        else:
            self.offer_amt.set_range(HandDisplay.DEFAULT_OFFER_AMT,
                                     HandDisplay.DEFAULT_OFFER_AMT)
            self.offer_amt.set_value(HandDisplay.DEFAULT_OFFER_AMT)
            self.offer_btn.set_sensitive(False)

    def on_toggle(self, widget):
        self.update_offer_ui()

    def on_selection(self, widget):
        self.selected_comm = self.comm_tab.get_selected_card()

        print(f'SELECTED {self.selected_comm.name.title()!r}')
        self.update_offer_ui()

    def on_offer(self, widget):
        offer_size = self.offer_amt.get_value_as_int()
        comm_amt = offer_size - len(self.wild)

        # take_cards() triggers on_selection when all cards of a type are
        # removed. This updates selected_comm, so save current card to
        # prevent this
        offered_card = self.selected_comm

        try:
            self.comm.take_cards(offered_card, comm_amt)
        except ValueError as e:
            return

        offer = [offered_card, *self.wild.active_wildcards]

        for wild_card in self.wild.active_wildcards:
            self.wild.take_card(wild_card)

        print(f'Sent offer of size {offer_size}',
              f'containing {", ".join([str(o) for o in offer])}')

        self.update_offer_ui()

    def on_quick_offer(self, widget, path, column):
        comm_amt = self.comm.get_amount(self.selected_comm)

        if comm_amt < HandDisplay.MIN_OFFER_AMT:
            return

        print(f'Sent offer of size {comm_amt}',
              f'containing {self.selected_comm!s}')

        self.comm.take_cards(self.selected_comm, comm_amt)