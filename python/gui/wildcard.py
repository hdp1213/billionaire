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

        self.radio_buttons = {}

        def new_wildcard_button(card_id):
            label = card_id.name.replace('_', ' ').title()
            self.radio_buttons[card_id] = Gtk.RadioButton(label=label)

        new_wildcard_button(CardID.BILLIONAIRE)
        new_wildcard_button(CardID.TAX_COLLECTOR)

        self.no_button = Gtk.RadioButton(label='None')
        self.no_button.set_sensitive(False)
        self.attach(self.no_button, 1, 1, 1, 1)

        for row, btn in enumerate(self.radio_buttons.values(), start=2):
            btn.join_group(self.no_button)
            btn.set_sensitive(False)
            self.attach(btn, 1, row, 1, 1)

        self.set_row_spacing(5)
        self.set_name('wildcards')

    def add_wildcard(self, card_id):
        self.no_button.set_active(True)
        self.no_button.set_sensitive(True)

        btn = self.radio_buttons.get(card_id)
        if btn is not None:
            btn.set_sensitive(True)

    def take_wildcard(self, card_id):
        self.no_button.set_active(True)
        if not self._cards:
            self.no_button.set_sensitive(False)

        btn = self.radio_buttons.get(card_id)
        if btn is not None:
            btn.set_sensitive(False)

    def clear_all(self):
        self.clear_cards()
        self.no_button.set_active(True)
        self.no_button.set_sensitive(False)

        for btn in self.radio_buttons.values():
            btn.set_sensitive(False)

    def on_add_new_cards(self, card_id, add_amt):
        self.add_wildcard(card_id)

    def on_remove_cards(self, card_id):
        self.take_wildcard(card_id)

    def is_valid_add(self, card_id, add_amt):
        return card_id in Wildcards.SET

    def is_valid_take(self, card_id, take_amt):
        return card_id in Wildcards.SET

    def connect_radio_buttons(self, func):
        for btn in self.radio_buttons.values():
            btn.connect('toggled', func)

    @property
    def has_taxcollector(self):
        return (self.get_amount(CardID.TAX_COLLECTOR) > 0)

    @property
    def has_billionaire(self):
        return (self.get_amount(CardID.TAX_COLLECTOR) > 0)

    @property
    def active_wildcards(self):
        return [key for key, btn in self.radio_buttons.items()
                if btn.get_active()]

    def __len__(self):
        return len(self.active_wildcards)
