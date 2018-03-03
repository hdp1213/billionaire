import gi
gi.require_version('Gtk', '3.0')

from gi.repository import GLib, Gtk, GObject

from card import CardID
from hand import Hand, HandTable

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

        self.hand = Hand()

        self.hand_tab = HandTable(self.hand)
        self.hand_tab.connect('cursor-changed', self.on_selection)
        self.hand_tab.connect('row-activated', self.on_quick_offer)

        card_adj = Gtk.Adjustment(value=DEFAULT_OFFER_AMT,
                                  lower=DEFAULT_OFFER_AMT,
                                  upper=DEFAULT_OFFER_AMT,
                                  step_increment=1)
        self.card_amt = Gtk.SpinButton(adjustment=card_adj)
        self.card_amt.set_numeric(True)

        self.card_amt.connect('activate', self.on_offer)

        self.offer_btn = Gtk.Button(label='Offer')

        self.offer_btn.connect('clicked', self.on_offer)

        self.wild_bil = Gtk.CheckButton(label='Billionaire')
        self.wild_tax = Gtk.CheckButton(label='Tax Collector')

        self.wild_bil.connect('toggled', self.on_toggle)
        self.wild_tax.connect('toggled', self.on_toggle)

        self.hand_grid.attach(self.hand_tab, 1, 1, 1, 3)
        self.hand_grid.attach(self.wild_bil, 2, 1, 1, 1)
        self.hand_grid.attach(self.wild_tax, 2, 2, 1, 1)
        self.hand_grid.attach(self.card_amt, 2, 3, 1, 1)
        self.hand_grid.attach(self.offer_btn, 3, 3, 1, 1)

        self.hand_grid.set_column_spacing(15)
        self.hand_grid.set_row_spacing(4)

        self.add(self.hand_grid)

        self.show_all()

    # EVENT HANDLERS

    def on_selection(self, widget):
        self.selected_card = self.hand_tab.get_selected_card()

        print(f'Activated {self.selected_card}')
        if self.selected_card == CardID.INVALID:
            self.card_amt.set_range(DEFAULT_OFFER_AMT, DEFAULT_OFFER_AMT)
        else:
            self.card_amt.set_range(MIN_OFFER_AMT,
                                    self.hand.get_amount(self.selected_card))

    def on_offer(self, widget):
        card_amt = self.card_amt.get_value_as_int()
        print(f'Sent offer of {card_amt}x {self.selected_card}')

        self.hand.take_cards(self.selected_card, card_amt)

    def on_quick_offer(self, widget, row, dof):
        card_amt = self.hand.get_amount(self.selected_card)
        print(f'Sent offer of {card_amt}x {self.selected_card}...')

        self.hand.take_cards(self.selected_card, card_amt)

    def on_toggle(self, check_button):
        print(f'{check_button.get_label()}: {check_button.get_active()}')


if __name__ == '__main__':
    # Calling GObject.threads_init() is not needed for PyGObject 3.10.2+
    GObject.threads_init()

    ClientApplication()
    Gtk.main()
