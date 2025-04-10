from corpus import Corpus
from quality import compute_quality_for_corpus
from random import randint
from utils import write_classification_to_file, count_chars, add_dicts, subtract_dicts, count_uppercase, extract_links, parse_email


class Filter:

    limits = {
        "headers": 0,
        "domains": 0,
        "subject": 0,
        "uppercase_ratio": 0,
        "char_occ": 0,
        "links": 0,
    }
    sus_subject_keywords = {
        "fortune",
        "hiring",
        "home",
        "income",
        "wealth",
        "free",
        "lottery",
        "winner",
        "urgent",
        "offer",
        "cash",
        "prize",
        "credit",
        "loan",
        "investment",
        "money",
        "deal",
        "guarantee",
        "cheap",
        "discount",
        "limited",
        "exclusive",
    }
    sus_phrases = {"act now", "don’t miss out", "urgent response needed", "risk-free", "no cost"}
    sus_headers = set()
    sus_domains = set()
    sus_links = set()
    sus_uppercase_ratio = 1
    sus_char_occ = {"!": 0, ".": 0, "=": 0, "^": 0, "~": 0, ":": 0, "%": 0, "|": 0, "\n": 0, "\t": 0}

    def train(self, path):
        self.calculate_flags(path)
        self.calculate_limits(path)

    def test(self, path):
        corpus = Corpus(path)
        flags_dict = self.analyze_corpus(path)
        pred_dict = {}
        for email in corpus.emails():
            pred_dict[email[0]] = "SPAM" if self.is_spam(flags_dict[email[0]]) else "OK"
        write_classification_to_file(path, pred_dict)

    def calculate_flags(self, path):
        corpus = Corpus(path)
        count_spams = 0
        headers_spams = set()
        chars_spams = {}
        uppercase_ratios_spams = []
        count_hams = 0
        headers_hams = set()
        chars_hams = {}
        #
        for _, body in corpus.spams():
            count_spams += 1
            headers, content = parse_email(body)
            headers_spams.update(headers.keys())
            chars_spams = add_dicts(chars_spams, count_chars(body))
            uppercase_ratios_spams.append(count_uppercase(list(content)) / len(list(content)))
            self.sus_links.update(extract_links(body))
            if "From" in headers:
                domain = headers["From"].split("@")[-1].strip()
                self.sus_domains.add(domain)
        #
        for _, body in corpus.hams():
            count_hams += 1
            headers, content = parse_email(body)
            headers_hams.update(headers.keys())
            chars_hams = add_dicts(chars_hams, count_chars(body))
        #
        self.sus_headers = headers_spams - headers_hams
        self.sus_uppercase_ratio = sum(uppercase_ratios_spams) / len(uppercase_ratios_spams)
        self.sus_char_occ = {
            **{key: 0 for key in subtract_dicts(chars_spams, chars_hams).keys()},
            **{key: (value // count_hams) for key, value in chars_hams.items() if key in self.sus_char_occ},
        }

    def calculate_limits(self, path):
        corpus = Corpus(path)
        flags_dict = self.analyze_corpus(path)
        last_iteration = {"accuracy": 0, "limits": self.limits}
        #
        for _ in range(150):
            pred_dict = {}
            for email in corpus.emails():
                pred_dict[email[0]] = "SPAM" if self.is_spam(flags_dict[email[0]]) else "OK"
            accuracy = compute_quality_for_corpus(path, pred_dict)
            if accuracy < last_iteration["accuracy"]:
                self.limits = last_iteration["limits"]
            else:
                last_iteration = {"accuracy": accuracy, "limits": self.limits.copy()}
                for limit in self.limits.keys():
                    self.limits[limit] = max(0, self.limits[limit] + randint(-3, 3))

    def analyze_corpus(self, path):
        corpus = Corpus(path)
        flags_dict = {}
        #
        for email in corpus.emails():
            headers, content = parse_email(email[1])
            flags_dict[email[0]] = {
                "headers": self.check_headers(headers),
                "domains": self.check_domains(headers),
                "subject": self.check_subject(headers),
                "uppercase_ratio": self.check_uppercase_ratio(content),
                "char_occ": self.check_char_occ(content),
                "links": self.check_links(content),
            }
        #
        return flags_dict

    def is_spam(self, flags):
        strikes = 0
        for flag, value in flags.items():
            if value > self.limits[flag]:
                strikes += 1
        return strikes >= round(len(self.limits) / 3)

    def check_headers(self, headers):
        flags = 0
        for header in headers.keys():
            if header in self.sus_headers:
                flags += 1
        return flags

    def check_domains(self, headers):
        flags = 0
        if "From" in headers:
            domain = headers["From"].split("@")[-1].strip()
            if domain in self.sus_domains:
                flags += 1
        return flags

    def check_subject(self, headers):
        flags = 0
        if "Subject" in headers:
            subject = headers["Subject"].lower()
            if any(phrase in subject for phrase in self.sus_phrases):
                flags += 1
            for keyword in self.sus_subject_keywords:
                if keyword in subject:
                    flags += 1
            if all(word.isupper() for word in subject.split() if word.isalpha()):
                flags += 1
        return flags

    def check_uppercase_ratio(self, content):
        flags = 0
        if count_uppercase(list(content)) / len(list(content)) > self.sus_uppercase_ratio:
            flags += 1
        return flags

    def check_char_occ(self, content):
        flags = 0
        for char, limit in self.sus_char_occ.items():
            if content.count(char) > limit:
                flags += 1
        return flags

    def check_links(self, content):
        flags = 0
        for link in extract_links(content):
            if link in self.sus_links:
                flags += 1
        return flags
