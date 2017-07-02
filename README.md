# IRC-C
Simple command line IRC client using sockets

# Description
This is a simple command line IRC client. Right now, it has very little functionality, and is basically glorified telnet.

* Support for persistent configuration - saves options that you enter on first run. To connect to a different server, delete config and re-run.
* Automatically sends NICK and USER messages on connection.
* PONGs all PINGs.
* Password field in config doesn't actually do anything.

No other IRC commands are implemented, and all have to be done manually. Even normal chatting has to be done with PRIVMSG

The code is fugly, but this is more me making sure I understand how sockets and basic IRC protocol works, so I can work on something more interesting ( ͡° ͜ʖ ͡°)

# License

License is MIT. I'll put the LICENSE file in there sometime.
