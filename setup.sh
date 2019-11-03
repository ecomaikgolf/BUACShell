if [ ! -d ~/.config/uacshell ]; then
    mkdir ~/.config/uacshell
fi

if [ ! -e ~/.config/uacshell/reserva.txt ]; then
    touch ~/.config/uacshell/reserva.txt
fi

if [ ! -e ~/.config/uacshell/user.toml ]; then
    touch ~/.config/uacshell/user.toml
fi

uname="$(stat --format '%U' ~/.config/uacshell/user.toml)"
if [ ! "${uname}" = "root" ]; then
    chmod ~/.config/uacshell/user.toml -g -u -r
    sudo chown root:root ~/.config/uacshell/user.toml
fi

