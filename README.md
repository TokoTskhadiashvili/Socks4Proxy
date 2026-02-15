# Socks4 Proxy

### The reason behind this project

Sometimes IoT devices run on what I call `Minimal OS`. This type of OS often is a small `Android` variant which does not even have a package manager. Also, there are not many commands available in the shell even when the user is `root`. For example, in many cases, IoT devices lack `ping`, `arp`, `netstat` and even `ncat`/`nc`. This makes lateral movement and network reconnaissance almost impossible because tools like `nmap` can't be used.

On the other hand, it's possible to sideload `termux.apk` file and install tools inside the `termux` environment but the user can easily detect unwanted application on the system.

That's why I wrote socks4 server. This can be installed on the compromised device in order to scan internal network. The device is going to be a proxy between new targets and the attacker.

### How to set it up?

In order to use this Proxy, compile it with `gcc` compiler on `Linux` machine like this:

```bash
gcc main.c -o main.elf
```

Now, `main.elf` file can be used as a socks4 proxy.
