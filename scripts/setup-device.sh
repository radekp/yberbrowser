#!/bin/bash

# this script sets up so that you don't have to use passwords to login
# to device

d=`dirname $0`
. $d/common.sh

own=""
case $1 in
    --own)
        own="rm -f /root/.ssh/authorized_keys /home/user/.ssh/authorized_keys"
        ;;
esac

grep device /etc/hosts >/dev/null 2>&1 || die "add '192.168.2.15    device' to your /etc/hosts"

hosts="device 192.168.2.15"
for h in hosts; do
    grep "Host $h" ~/.ssh/config >/dev/null  2>&1 || printf "\nHost $h\n   StrictHostKeyChecking no\n" >> ~/.ssh/config
done

extrasdev="deb http://repository.maemo.org/extras-devel/ fremantle free non-free"

if [ -f $HOME/.ssh/id_rsa.pub ]; then
	key=`cat $HOME/.ssh/id_rsa.pub`
	ssh root@device <<EOF
$own
mkdir -p /root/.ssh
grep '$key' /root/.ssh/authorized_keys >/dev/null 2>&1  || echo '$key' >> /root/.ssh/authorized_keys
chmod -R 700 /root/.ssh

mkdir -p /home/user/.ssh
grep '$key' /home/user/.ssh/authorized_keys >/dev/null 2>&1  || echo '$key' >> /home/user/.ssh/authorized_keys
chmod -R 700 /home/user/.ssh
chown -R user:users /home/user/.ssh

if [ ! -f /etc/sudoers.d/99-sudoall ]; then
echo "user ALL = NOPASSWD: ALL" > /etc/sudoers.d/99-sudoall
update-sudoers
fi

passwd -d user
passwd -d root

grep '$extrasdev' /etc/apt/sources.list.d/hildon-application-manager.list >/dev/null 2>&1 || echo '$extrasdev' >>  /etc/apt/sources.list.d/hildon-application-manager.list
dpkg-query -S rsync >/dev/null 2>&1 || (apt-get update && apt-get -y install rsync)
EOF
else
    echo "Not adding your public key to device. You don't have one. use ssh-keygen"
fi
