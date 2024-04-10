#!/bin/bash

# arg1 : filename
TARGET="./"
NAME="$1"
LOCAL_PATH="$TARGET/$NAME"

# check if file exists
if [ ! -e "$LOCAL_PATH" ]; then
    echo "file not found : $LOCAL_PATH"
    exit 1
fi

# scp to remote server
SERVER="192.168.101.30"
USER="user"
PASS="user"
REMOTE_PATH="/mnt/jffs/usr/$NAME"

echo "sending file to $SERVER  :  $LOCAL_PATH ...-> $REMOTE_PATH"
scp -oHostKeyAlgorithms=ssh-rsa "$LOCAL_PATH" "$USER"@"$SERVER":"$REMOTE_PATH"
echo "upload done!!"

# sshして書き込み先のファイルを実行できるようにする
echo "ssh to $SERVER and chmod +x $REMOTE_PATH"
ssh -oHostKeyAlgorithms=ssh-rsa "$USER"@"$SERVER" chmod +x "$REMOTE_PATH"
echo "done!!"

# read -p "sshを実行します。よろしいですか (Y/N)？" ANSWER
# if [[ "$ANSWER" == "y" ]] || [[ "$ANSWER" == "Y" ]]; then
#     # ssh -oHostKeyAlgorithms=ssh-rsa "$USER"@"$SERVER" source dotconfig_source.bashrc
#     ssh -oHostKeyAlgorithms=ssh-rsa "$USER"@"$SERVER"
# fi

