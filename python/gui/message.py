import gi
gi.require_version('Gtk', '3.0')

from gi.repository import Gio, GObject, GLib
from json.decoder import JSONDecodeError

from card_location import CardLocation
from command import Command, CommandList
from wildcard import Wildcards

TCP_IP = '127.0.0.1'
TCP_PORT = 5555
ADDR = (TCP_IP, TCP_PORT)


class MessagePasser(GObject.Object):
    """A test, first and foremost"""
    READ_BYTES = 1024

    def __init__(self):
        self.cancellable = Gio.Cancellable()

        self.output = None
        self.input = None

        self.client = Gio.SocketClient.new()

        self.hand = None
        self.offers = None

    def connect_objects(self, hand, offers):
        self.hand = hand
        self.offers = offers

        self.hand.connect('new-offer', self.on_new_offer)
        self.offers.connect('cancel-offer', self.on_cancel_offer)

    def send_command(self, command):
        if self.output is None:
            return

        command_list = CommandList(command)

        self.output.write(command_list.to_json().encode('utf-8'))

    def run(self):
        socket_addr = Gio.InetSocketAddress.new_from_string(*ADDR)
        self.client.connect_async(socket_addr, self.cancellable,
                                  self.on_connection)

    def add_cards_to_gui(self, card_loc):
        for card_id, card_amt in card_loc:
            if card_id in Wildcards.SET:
                self.hand.wild.add_cards(card_id, card_amt)
            else:
                self.hand.comm_data.add_cards(card_id, card_amt)

    def on_connection(self, source_object, result, *user_data):
        print('Connected to server')
        self.conn = source_object.connect_finish(result)
        self.output = self.conn.get_output_stream()
        self.input = self.conn.get_input_stream()

        self.input.read_bytes_async(MessagePasser.READ_BYTES,
                                    GLib.PRIORITY_DEFAULT, self.cancellable,
                                    self.on_read)

    def on_read(self, source_object, result, *user_data):
        """Receive commands from the server"""
        data = source_object.read_bytes_finish(result).get_data()

        try:
            self.received_cmds = CommandList.from_bytes(data)
        except UnicodeDecodeError as e:
            print('Received invalid unicode')
            return
        except JSONDecodeError as e:
            print('Received invalid JSON')
            print(data)
            return

        print(f'RECEIVED {self.received_cmds!r}')

        if Command.JOIN in self.received_cmds:
            join_cmd = self.received_cmds[Command.JOIN]
            self.id = join_cmd.client_id

        if Command.START in self.received_cmds:
            print('Starting game...')
            start_cmd = self.received_cmds[Command.START]

            hand = CardLocation.from_json(start_cmd.hand)
            self.add_cards_to_gui(hand)

        if Command.SUCCESSFUL_TRADE in self.received_cmds:
            trade_cmd = self.received_cmds[Command.SUCCESSFUL_TRADE]

            trade = CardLocation.from_json(trade_cmd.cards)

            self.offers.data.remove_offer(len(trade))
            self.add_cards_to_gui(trade)

        if Command.CANCELLED_OFFER in self.received_cmds:
            cancelled_cmd = self.received_cmds[Command.CANCELLED_OFFER]

            offer = CardLocation.from_json(cancelled_cmd.cards)
            self.add_cards_to_gui(offer)

        if Command.BOOK_EVENT in self.received_cmds:
            book_cmd = self.received_cmds[Command.BOOK_EVENT]

            if book_cmd.event == Command.NEW_OFFER:
                owner_id = book_cmd.participants[0]
                self.offers.data.add_offer(book_cmd.card_amt, owner_id)

            # Either CANCEL_OFFER or SUCCESSFUL_TRADE
            else:
                self.offers.data.remove_offer(book_cmd.card_amt)

        if Command.FINISH in self.received_cmds:
            print('Stopping game...')
            self.hand.clear_all()
            self.offers.clear_all()

        if Command.BILLIONAIRE in self.received_cmds:
            billionaire_cmd = self.received_cmds[Command.BILLIONAIRE]

            if billionaire_cmd.winner_id == self.id:
                print('A winner is you!')
            else:
                print(f'Bad luck, {billionaire_cmd.winner_id} won.')

        if Command.ERROR in self.received_cmds:
            pass

        # Continually add this
        source_object.read_bytes_async(MessagePasser.READ_BYTES,
                                       GLib.PRIORITY_DEFAULT, self.cancellable,
                                       self.on_read)

    def on_new_offer(self, widget, offer):
        print(f'NEW_OFFER: {offer}')

        new_offer_command = Command(Command.NEW_OFFER, cards=offer.to_dict())

        self.offers.data.add_offer(len(offer), self.id)

        self.send_command(new_offer_command)

    def on_cancel_offer(self, widget, offer_amt):
        print(f'CANCEL_OFFER {offer_amt}')

        cancel_offer_command = Command(Command.CANCEL_OFFER,
                                       card_amt=offer_amt)

        if self.offers.data.owner_id_of(offer_amt) == self.id:
            self.offers.data.remove_offer(offer_amt)

        self.send_command(cancel_offer_command)
