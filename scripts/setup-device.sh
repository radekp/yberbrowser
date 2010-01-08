#!/bin/bash

scriptdir=`dirname $0`

. $scriptdir/common-vars.sh

grep device /etc/hosts 2>&1 >/dev/null || die "add '192.168.2.15    device' to your /etc/hosts"

grep "Host device" ~/.ssh/config 2>&1 >/dev/null || printf "\nHost device\n   StrictHostKeyChecking no\n" >> ~/.ssh/config

if [ -f $HOME/.ssh/id_rsa.pub ]; then
	key=`cat $HOME/.ssh/id_rsa.pub`
	ssh root@device <<EOF
mkdir -p /root/.ssh
grep '$key' /root/.ssh/authorized_keys 2>&1 >/dev/null || echo '$key' >> /root/.ssh/authorized_keys
chmod -R 700 /root/.ssh

mkdir -p /home/user/.ssh
grep '$key' /home/user/.ssh/authorized_keys 2>&1 >/dev/null || echo '$key' >> /home/user/.ssh/authorized_keys
chmod -R 700 /home/user/.ssh
chown -R user:users /home/user/.ssh
EOF
fi

