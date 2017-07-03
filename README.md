# irC
Simple command line IRC client using sockets

# Description
This is a simple command line IRC client. Right now, it has very little functionality, and is basically glorified telnet.

* Support for persistent configuration - saves options that you enter on first run.
* Load any config file with `./IRC <config file>`. If it does not exist, it will create it. If config file is not specified, 
`./config` is loaded by default
* Automatically sends NICK and USER messages on connection.
* PONGs all PINGs.
* Password field in config doesn't actually do anything.
* Real name support as a config option. If no real name specified, will use username in real name field (what it did before)

No other IRC commands are implemented, and all have to be done manually. Even normal chatting has to be done with PRIVMSG

The code is fugly, but this is more me making sure I understand how sockets and basic IRC protocol works, so I can work on something more interesting ( ͡° ͜ʖ ͡°)

# License

License is MIT.
