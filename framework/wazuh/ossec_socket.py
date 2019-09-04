

# Copyright (C) 2015-2019, Wazuh Inc.
# Created by Wazuh, Inc. <info@wazuh.com>.
# This program is free software; you can redistribute it and/or modify it under the terms of GPLv2

from wazuh.exception import WazuhError, WazuhInternalError
from wazuh import common
import socket
from json import dumps, loads
from struct import pack, unpack


class OssecSocket:

    MAX_SIZE = 65536

    def __init__(self, path):
        self.path = path
        self._connect()

    def _connect(self):
        try:
            self.s = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
            self.s.connect(self.path)
        except Exception as e:
            raise WazuhInternalError(1013, extra_message=str(e))

    def close(self):
        self.s.close()

    def send(self, msg_bytes):
        if not isinstance(msg_bytes, bytes):
            raise WazuhError(1104, extra_message="Type must be bytes")

        try:
            sent = self.s.send(pack("<I", len(msg_bytes)) + msg_bytes)
            if sent == 0:
                raise WazuhInternalError(1014, extra_message="Number of sent bytes is 0")
            return sent
        except Exception as e:
            raise WazuhInternalError(1014, extra_message=str(e))

    def receive(self):

        try:
            size = unpack("<I", self.s.recv(4, socket.MSG_WAITALL))[0]
            return self.s.recv(size, socket.MSG_WAITALL)
        except Exception as e:
            raise WazuhInternalError(1014, extra_message=str(e))


class OssecSocketJSON(OssecSocket):

    MAX_SIZE = 65536

    def __init__(self, path):
        OssecSocket.__init__(self, path)

    def send(self, msg):
        return OssecSocket.send(self, dumps(msg).encode())

    def receive(self):
        response = loads(OssecSocket.receive(self).decode())

        if 'error' in response.keys():
            if response['error'] != 0:
                raise WazuhError(response['error'], extra_message=response['message'], cmd_error=True)
            else:
                return response['data']
