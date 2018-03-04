import gi
gi.require_version('Gtk', '3.0')

from gi.repository import GLib, Gtk, GObject

from card import CardID
from commodity import CommodityData, CommodityTable
from wildcard import Wildcards

DEFAULT_OFFER_AMT = 0
MIN_OFFER_AMT = 2


class ClientApplication(Gtk.Window):
    """Client frontend GUI for Billionaire playing"""
    def __init__(self):
        super(ClientApplication, self).__init__(default_height=50,
                                                default_width=200)
        self.connect('delete-event', Gtk.main_quit)

        self.set_title('Billionaire')

        self.selected_card = CardID.INVALID

        self.hand_grid = Gtk.Grid()

        self.comm_data = CommodityData()

        self.comm_tab = CommodityTable(self.comm_data)
        self.comm_tab.connect('cursor-changed', self.on_selection)
        self.comm_tab.connect('row-activated', self.on_quick_offer)

        card_adj = Gtk.Adjustment(value=DEFAULT_OFFER_AMT,
                                  lower=DEFAULT_OFFER_AMT,
                                  upper=DEFAULT_OFFER_AMT,
                                  step_increment=1)
        self.card_amt = Gtk.SpinButton(adjustment=card_adj)
        self.card_amt.set_numeric(True)
        self.card_amt.connect('activate', self.on_offer)

        self.wilds = Wildcards()

        self.offer_btn = Gtk.Button(label='Offer')
        self.offer_btn.connect('clicked', self.on_offer)
        self.offer_btn.set_sensitive(False)

        self.hand_grid.attach(self.comm_tab, 1, 1, 1, 3)
        self.hand_grid.attach(self.wilds, 2, 1, 1, 2)
        self.hand_grid.attach(self.card_amt, 2, 3, 1, 1)
        self.hand_grid.attach(self.offer_btn, 3, 3, 1, 1)

        self.hand_grid.set_column_spacing(15)
        self.hand_grid.set_row_spacing(5)

        self.hand_grid.set_margin_start(5)
        self.hand_grid.set_margin_end(5)
        self.hand_grid.set_margin_top(5)
        self.hand_grid.set_margin_bottom(5)
        self.hand_grid.set_name('hand-grid')

        self.add(self.hand_grid)

        self.show_all()

    def invalid_selection(self):
        self.card_amt.set_range(DEFAULT_OFFER_AMT, DEFAULT_OFFER_AMT)
        self.offer_btn.set_sensitive(False)

    def valid_selection(self):
        self.card_amt.set_range(MIN_OFFER_AMT,
                                self.comm_data.get_amount(self.selected_card))
        self.offer_btn.set_sensitive(True)

    # EVENT HANDLERS

    def on_selection(self, widget):
        self.selected_card = self.comm_tab.get_selected_card()

        print(f'Activated {self.selected_card}')
        if self.selected_card == CardID.INVALID:
            self.invalid_selection()
        else:
            self.valid_selection()

    def on_offer(self, widget):
        card_amt = self.card_amt.get_value_as_int()
        offered_card = self.selected_card

        try:
            self.comm_data.take_cards(offered_card, card_amt)
        except ValueError as e:
            return
        else:
            print(f'Sent offer of {card_amt}x {offered_card}')

    def on_quick_offer(self, widget, row, dof):
        card_amt = self.comm_data.get_amount(self.selected_card)

        if card_amt < MIN_OFFER_AMT:
            return

        print(f'Sent offer of {card_amt}x {self.selected_card}...')

        self.comm_data.take_cards(self.selected_card, card_amt)


if __name__ == '__main__':
    # Calling GObject.threads_init() is not needed for PyGObject 3.10.2+
    GObject.threads_init()

    ClientApplication()
    Gtk.main()
