# Billionaire Messaging Protocol
_16/11/17, Harry Poulter_

Billionaire, like any online game worth its salt, works by sending
messages between the server and its clients. The format these message
packets take defines a _messaging protocol_.

To keep a client implementation as open-ended as possible, the
Billionaire server uses the ubiqutous JSON data-interchange format to
describe its message packets.

The server works by passing a string representation of a JSON object as
bytes through a TCP connection to its clients. Conversely, any client
implementation must send a JSON string as bytes to the server in order
for it to be considered correctly implemented.


## The command packet
The fundamental message packet that is sent between server and client.
Takes the following form:

```json
{
  "commands":
  [
    /* <cmd_1> */,
    /* <cmd_2> */,
    /* ... */
  ]
}
```

Here,
 - `<cmd_X>`: a command object as outlined below.


## Command objects
Command objects have the generic form of

```json
{
  "command": /* <command_type> */,
  /* <command_attributes> */
  /* ... */
}
```

where
 - `<command_type>`: a string identifying what command the object
corresponds to.
 - `<command_attributes>`: command-specific key-value pairs.


## Command types and their attributes

### Server-to-client commands
Commands sent by the server to the client.

#### `JOIN`:
Adds a client to the current active roster of players. Issued by the
server as a one-sided handshake (?) to give the client its unique ID
that will be used throughout the game, and implicitly confirm the client
can communicate with the server.
 - `client_id`: client ID used for later identification.

#### `START`:
Starts the game when enough clients have joined. Contains the starting
hands for each client.
 - `hand`: array of cards objects.

#### `SUCCESSFUL_TRADE`:
Gives a client a set of cards corresponding to a successful trade.
 - `cards`: array of cards objects.
 - `owner_id`: ID of the cards' original owner which the client can use
at its own discretion.

#### `CANCELLED_OFFER`:
Returns to the client a cancelled offer they had previously added.
 - `cards`: array of cards objects.

#### `BOOK_EVENT`:
Sent to all non-participants when a book event occurs, _i.e._ something
that changes the group of current offers (the _book_).
 - `event`: either `NEW_OFFER`, `CANCEL_OFFER` or `SUCCESSFUL_TRADE`.
 - `card_amt`: amount of cards in the trade event.
 - `participants`: array of client ID(s) that featured as part of the
event.

#### `FINISH`:
Finishes the game, either when a client disconnects or when the game is
won.

#### `ERROR`:
Notifies a client that an error occurred during processing of a command
sent by the client. There will be a variety of different errors
associated with this particular command that will be defined at a later
date.
 - `errno`: machine-readable integer describing the error.
 - `what`: human-readable string describing the error.


### Client-to-server commands
Commands sent by the client to the server.

#### `NEW_OFFER`:
Instigate an offer by asking for a given number of cards. Only the
server knows the types of the cards given, all other clients only know
the number. Upon successful trade (another client matches the ask), both
sets of cards are exchanged to the respective clients.
 - `cards`: array of cards objects.

#### `CANCEL_OFFER`:
Cancels the corresponding `NEW_OFFER` for the given `card_amt`.
 - `card_amt`: amount of cards in original offer.


## Other objects

### Cards object
Takes the simple form of

```json
{
  "id": /* <id> */,
  "amt": /* <amount> */,
  "val": /* <value> */
}
```

where
 - `<id>`: the card's ID. All cards of the same type (same commodity, or
same special card) have the same card ID. A card ID is implemented as an
integer. An invalid card will always have an ID of `-1`.
 - `<amount>`: the number of cards packed in the object. Must be greater
than zero.
 - `<value>`: (optional) the value in dollars of the card. Note that
this will only ever be sent to the client for their own purposes. The
server does not require the client to send the value back, and its
presence does not affect trading.

All fields in the cards object must be specified as integers.


## Error codes

When a command is given to the Billionaire server, it must satisfy both
the protocol defined in this document, as well as dynamic conditions
resulting from a game being played. If at any given point such a command
fails any of these checks, an `ERROR` command is sent from the server
back to the client to notify the client of the check it has failed.

Following are the different error codes that can be received from the
server:

 - `EJSON`: sent JSON cannot be parsed. Takes many different values as
this is handled by the `json-c` library.
 - `EJSONVAL`: command object does not contain a field needed by the
server.
 - `EJSONTYPE`: JSON object is not the desired type.
 - `EBADCMDNAME`: command object has an invalid command name.
 - `EBADCMDOBJ`: command object does not have a 'command' field.
 - `ECANEMPTY`: `CANCEL_ORDER` object cancelling an empty offer.
 - `ECANPERM`: `CANCEL_ORDER` object cancelling an offer not owned by
the client.
 - `EOFFEROVER`: `NEW_OFFER` object attempting to overwrite client's
offer already in book.
 - `ENOOFFER`: `NEW_OFFER` object does not contain any cards.
 - `ESMALLOFFER`: `NEW_OFFER` object does not contain _enough_ cards.
 - `EHANDSUBSET`:  `NEW_OFFER` object does not contain cards that are
present in the client's hand, to prevent cheating ;).
 - `EUNIQCOMMS`: `NEW_OFFER` object contains more than one unique
commodity.
 - `EUNIQWILDS`: `NEW_OFFER` object contains more than one unique
wildcard.
 - `ECARDRM`: too many cards have been requested to be removed from a
`card_location` object.
