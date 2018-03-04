import asyncio

TCP_IP = '127.0.0.1'
TCP_PORT = 5555
ADDR = (TCP_IP, TCP_PORT)


class MessagePasser():
    """A test, first and foremost"""
    def __init__(self, hand):
        hand.connect('new_offer', self.on_new_offer)

    def on_new_offer(self, widget, offer):
        print(f'NEW_OFFER: {offer}')
