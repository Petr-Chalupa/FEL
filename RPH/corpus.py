from os import listdir
from os.path import isfile, join
from utils import read_classification_from_file


class Corpus:
    def __init__(self, directory):
        self.directory = directory

    def get_class(self, fname):
        classifications = read_classification_from_file(join(self.directory, "!truth.txt"))
        return classifications[fname]

    def is_spam(self, fname):
        return self.get_class(fname) == "SPAM"

    def is_ham(self, fname):
        return self.get_class(fname) == "OK"

    def emails(self):
        file_names = [f for f in listdir(self.directory) if (isfile(join(self.directory, f)) and not f.startswith("!"))]
        for fname in file_names:
            with open(join(self.directory, fname), "rt", encoding="utf-8") as file:
                yield (fname, file.read())

    def spams(self):
        for fname, body in self.emails():
            if self.is_spam(fname):
                yield (fname, body)

    def hams(self):
        for fname, body in self.emails():
            if self.is_ham(fname):
                yield (fname, body)
