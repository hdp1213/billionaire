import abc
from functools import wraps
import json


class JSONInterface(metaclass=abc.ABCMeta):
    """A virtual class that can be"""
    @abc.abstractmethod
    def to_dict(self):
        return NotImplemented

    def to_json(self):
        return json.dumps(self.to_dict, separators=(',', ':'))
