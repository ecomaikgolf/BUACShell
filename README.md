# BUACShell [![Build Status](https://travis-ci.com/ecomaikgolf/BUACShell.svg?branch=master)](https://travis-ci.com/ecomaikgolf/BUACShell) ![GitHub stars](https://img.shields.io/github/stars/ecomaikgolf/BUACShell?style=social)
This project aims to provide an unofficial terminal application for the University of Alicante library

## Installation
```
git clone https://github.com/ecomaikgolf/BUACShell.git
cd BUACShell
sudo make install
```
Dependencies: 
 * libcurl

Operating Systems:
 * GNU/Linux: Supported
 * Linux Subsystem for Windows: Not tested

## Updating
```
cd BUACShell
git pull -f
sudo make install
```
If you get `<curl/curl.h> does not exist..` error, try installing `libcurl4-gnutls-dev` package or similar provider. (Ubuntu) `sudo apt install  libcurl4-gnutls-dev`

If you get `make: command not found` error, try installing `make` package. (Ubuntu) `sudo apt install make`


## Usage
```
Usage: bua COMMAND

 Command list:
  renew [-f]  Renews all non-reserved books
  list        Lists reserved books from local cache

 Parameter list:
  [-f] Force credential input
 ```

## Features
* Lightweight (~70K)
* Fast (C++)
* Easy build from source (make)
* Non bloated with dependencies (libcurl)
* Scriptable (cron)
* Autologin protected with system permissions
* Open Source!

## FAQ
### Why do I need sudo to compile?
In order to disallow non-privileged users from reading `~/.config/uacshell/user.toml` we remove read/write permissions from everyone except root-level users.

The program in order to read the file is compiled with sudo privileges (as `ping` does) and only uses them when necessary (escalate/drop privileges), in this case when it has to read the DNI/NIP. This type of binaries are safe agains non-privileged binary instrumentation attacks.

As the file is stored in `home`, modern distributions (as Ubuntu) allow transparent encryption for `home`, so the credentials are safe even at a physical level.

This way, you benefit from a passwordless login while your credentials stay **a bit more** safe.

Also we need sudo to install to move the binary to `/usr/bin`
### What happens if the library software changes?
BUACShell won't work.

The library software is expected to be renewed during 2020, we'll try to release an update if possible.

### What happens if someone reserved one of my books?
It appears if you do `bua renew` and that situation occurs.

If you are using cron jobs or some kind of automatic scripting, BUACShell main function returns the number of non-renewed books in a error free execution.

### Can I renew my books each minute?
Flooding their servers with dummy requests could result in a Denial Of Service and they can block you from accessing the library webpage. Anyway you can't renew books more than once per day.

Please do not flood their servers and consider renewing books not more than once per day. If you need to list the books consider using `bua list` that uses local cache.

### Some file is missing
Perhaps you removed/moved `~/.config/uacshell/`, try to do `sudo make install` again and look for errors.

### I have error X
Contact us.

## Colaborate
You can colaborate in several ways:
* Submitting issues [here](https://github.com/ecomaikgolf/BUACShell/issues)
* Pull requests [here](https://github.com/ecomaikgolf/BUACShell/pulls)
  * New features
  * Bug fixes
* Spread this project

## Developers
**Ernesto Martínez García** [1]
 * ecomaikgolf@protonmail.com
 * [ecomaikgolf.com](ecomaikgolf.com)
 * ![Twitter Follow](https://img.shields.io/twitter/follow/ecomaikgolf?style=social)
 * ![GitHub followers](https://img.shields.io/github/followers/ecomaikgolf?style=social)

**Ignacio Encinas Rubio** [1]
 * ier11@alu.ua.es
 * ![GitHub followers](https://img.shields.io/github/followers/IEncinas10?style=social)
 
[1] Computer Science, University of Alicante Polytechnic School

# UACShell
![](https://drive.ecomaikgolf.com/Public/UACShell/banner.png)   

BUACShell is a small module from a bigger project called UACShell that aims to provide a terminal interface for the University of Alicante virtual campus (UACloud)

UACShell provides a quicker and easier way to do daily tasks (downloading materiales, checking marks, notifications...) rather than being a complete substitute of the web interface (payments, administrative tasks...).

It manages authentication process automatically so you just have to setup username/password once, then just use `uacloud list assignments` `uacloud list materials X` `uacloud download` ... anytime you want

This project is being developed as closed-source until we solve some burocracy.

BUACShell will be merged with UACShell on a single project.

# BUACShell installation video
[![asciicast](https://asciinema.org/a/cr0xmAtLnrxnUdh4k3nNG3dpk.svg)](https://asciinema.org/a/cr0xmAtLnrxnUdh4k3nNG3dpk)

