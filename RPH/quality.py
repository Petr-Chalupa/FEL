from sys import argv
from os import path, remove
from inspect import getmembers, isclass
from importlib import import_module
from utils import read_classification_from_file
from confmat import BinaryConfusionMatrix


def quality_score(tp, tn, fp, fn):
    q = (tp + tn) / (tp + tn + 10 * fp + fn)
    return q


def compute_quality_for_corpus(corpus_dir, pred_dict=None):
    truth_dict = read_classification_from_file(path.join(corpus_dir, "!truth.txt"))
    pred_dict = pred_dict if pred_dict != None else read_classification_from_file(path.join(corpus_dir, "!prediction.txt"))
    #
    bcm = BinaryConfusionMatrix("SPAM", "OK")
    bcm.compute_from_dicts(truth_dict, pred_dict)
    return quality_score(**bcm.as_dict())


if __name__ == "__main__":
    if len(argv) < 4:
        raise ValueError("Not enough arguments passed!\n" f"\tUse: {argv[0]} <filter_module> <corpus_train> <corpus_test>\n" f"\tEx.: {argv[0]} simplefilters ./data/1 ./data/2")

    module = import_module(argv[1])
    corpus_train_dir = argv[2]
    corpus_test_dir = argv[3]

    classes = [(name, obj) for name, obj in getmembers(module) if isclass(obj)]
    for cname, cobj in classes:
        if not cname.endswith("Filter"):
            continue
        #
        print(f"Testing filter <{cname}>")
        f = cobj()
        f.train(corpus_train_dir)
        f.test(corpus_test_dir)
        #
        q = compute_quality_for_corpus(corpus_test_dir)
        print(f"\tScore: { round(q * 100, 2) } %")
        #
        remove(path.join(corpus_test_dir, "!prediction.txt"))
