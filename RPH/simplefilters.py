from utils import write_classification_to_file
from corpus import Corpus
import random


class NaiveFilter:
    def train(self, path):
        pass

    def test(self, path):
        dictionary = {}
        for fname, body in Corpus(path).emails():
            dictionary[fname] = "OK"
        write_classification_to_file(path, dictionary)


class ParanoidFilter:
    def train(self, path):
        pass

    def test(self, path):
        dictionary = {}
        for fname, body in Corpus(path).emails():
            dictionary[fname] = "SPAM"
        write_classification_to_file(path, dictionary)


class RandomFilter:
    def train(self, path):
        pass

    def test(self, path):
        dictionary = {}
        for fname, body in Corpus(path).emails():
            dictionary[fname] = random.choice(["OK", "SPAM"])
        write_classification_to_file(path, dictionary)
