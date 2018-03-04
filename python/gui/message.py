import asyncio

TCP_IP = '127.0.0.1'
TCP_PORT = 5555
ADDR = (TCP_IP, TCP_PORT)


class MessagePasser():
    """A test, first and foremost"""
    def __init__(self, hand, offers):
        hand.connect('new-offer', self.on_new_offer)
        offers.connect('cancel-offer', self.on_cancel_offer)

    def on_new_offer(self, widget, offer):
        print(f'NEW_OFFER: {offer}')

    def on_cancel_offer(self, widget, offer_amt):
        print(f'CANCEL_OFFER {offer_amt}')
