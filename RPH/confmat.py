class BinaryConfusionMatrix:
    def __init__(self, pos_tag, neg_tag):
        self.pos_tag = pos_tag
        self.neg_tag = neg_tag
        self.tp = 0
        self.fp = 0
        self.tn = 0
        self.fn = 0

    def as_dict(self):
        return {"tp": self.tp, "fp": self.fp, "tn": self.tn, "fn": self.fn}

    def update(self, truth, prediction):
        if truth != self.pos_tag and truth != self.neg_tag or prediction != self.pos_tag and prediction != self.neg_tag:
            raise ValueError
        #
        if truth == self.pos_tag:
            if truth == prediction:
                self.tp += 1
            else:
                self.fn += 1
        if truth == self.neg_tag:
            if truth == prediction:
                self.tn += 1
            else:
                self.fp += 1

    def compute_from_dicts(self, truth_dict, pred_dict):
        for key in truth_dict:
            self.update(truth_dict[key], pred_dict[key])
