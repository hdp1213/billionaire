#!/usr/bin/env python

from base import BotDriver
import bots

TCP_IP = '127.0.0.1'
TCP_PORT = 5555
ADDR = (TCP_IP, TCP_PORT)


def main():
    driver = BotDriver(ADDR, bots.DumbBot)
    driver.run()

if __name__ == '__main__':
    exit(main())
