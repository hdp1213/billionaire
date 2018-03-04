from collections import Counter
import enum
import gi
gi.require_version('Gtk', '3.0')

from gi.repository import Gtk

from card import CardID, CardData


class Wildcards(Gtk.Grid, CardData):
    """Container for wildcard options"""
    SET = {CardID.BILLIONAIRE,
           CardID.TAX_COLLECTOR}

    def __init__(self):
        Gtk.Grid.__init__(self)
        CardData.__init__(self)

        self.check_buttons = {}

        def new_wildcard_button(card_id):
            label = card_id.name.replace('_', ' ').title()
            self.check_buttons[card_id] = Gtk.CheckButton(label=label)

        new_wildcard_button(CardID.BILLIONAIRE)
        new_wildcard_button(CardID.TAX_COLLECTOR)

        for row, btn in enumerate(self.check_buttons.values(), start=1):
            btn.connect('toggled', self.on_toggle)
            btn.set_sensitive(False)
            self.attach(btn, 1, row, 1, 1)

        self.set_row_spacing(5)
        self.set_name('wildcards')

        self.add_card(CardID.BILLIONAIRE)
        self.add_card(CardID.TAX_COLLECTOR)

    def add_wildcard(self, card_id):
        btn = self.check_buttons.get(card_id)

        if btn is not None:
            btn.set_active(False)
            btn.set_sensitive(True)

    def take_wildcard(self, card_id):
        btn = self.check_buttons.get(card_id)

        if btn is not None:
            btn.set_active(False)
            btn.set_sensitive(False)

    def on_add_new_cards(self, card_id, add_amt):
        self.add_wildcard(card_id)

    def on_remove_cards(self, card_id):
        self.take_wildcard(card_id)

    def is_valid_add(self, card_id, add_amt):
        return card_id in Wildcards.SET

    def is_valid_take(self, card_id, take_amt):
        return card_id in Wildcards.SET

    def on_toggle(self, check_btn):
        print(f'TOGGLE {check_btn.get_label()!r}: {check_btn.get_active()}')

    def connect_check_buttons(self, func):
        for btn in self.check_buttons.values():
            btn.connect('toggled', func)

    @property
    def has_taxcollector(self):
        return (self.get_amount(CardID.TAX_COLLECTOR) > 0)

    @property
    def has_billionaire(self):
        return (self.get_amount(CardID.TAX_COLLECTOR) > 0)

    @property
    def active_wildcards(self):
        return [key for key, btn in self.check_buttons.items()
                if btn.get_active()]

    def __len__(self):
        return len(self.active_wildcards)