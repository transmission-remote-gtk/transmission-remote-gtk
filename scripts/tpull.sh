#!/usr/bin/env bash

# This is a really simple script intended to be run as an action
# from transmission-remote-gtk. It uses rsync to fetch
# a torrent/torrents to a local directory (or put it somewhere
# remote and run it using ssh to there).

# It also shows how we can call transmission-remote with
# connection details to find information about a torrent or
# manipulate it.

# Example
# gnome-terminal -e "tpull.sh %{hostname} %{port} %{username}:%{password} %{id}[,] /srv/incoming/"

if [ -z "$5" ]; then
  echo "usage: <host> <port> <user:pass> <id> <dest>"
  exit 1
fi

HOST=$1
TPORT=$2
TAUTH=$3
IDS=$4
DEST=$5

echo $IDS | sed "s/,/\n/g" | while read id; do
  DETAILS=$(transmission-remote $HOST:$TPORT -n $TAUTH -t $id -i)

  if [ $? -ne 0 ]; then
    read
    exit 1
  fi

  LOCATION=$(echo "$DETAILS" | egrep '^\s+Location:' | cut -c 13-)
  NAME=$(echo "$DETAILS" | egrep '^\s+Name:' | cut -c 9-)

  if [ -z "$LOCATION" -o -z "$NAME" ]; then
    continue
  fi

  FULLPATH="$LOCATION/$NAME"

  echo "Syncing $FULLPATH ..."
  rsync -avPs "$HOST:$FULLPATH" "$DEST"
done
