from os import path


def read_classification_from_file(file_path):
    email_dict = {}
    with open(file_path, "rt", encoding="utf-8") as file:
        for line in file.readlines():
            values = line.split()
            email_dict[values[0]] = values[1]
    return email_dict


def write_classification_to_file(file_path, dictionary):
    with open(path.join(file_path, "!prediction.txt"), "w+", encoding="utf-8") as file:
        for key in dictionary:
            file.write(key + " " + dictionary[key] + "\n")


def count_chars(string):
    freq = {}
    for char in string:
        if char in freq:
            freq[char] += 1
        else:
            freq[char] = 1
    return freq


def count_uppercase(string):
    uppercase = 0
    for char in string:
        if char.upper() == char:
            uppercase += 1
    return uppercase


def add_dicts(first, second):
    for key, value in second.items():
        if key in first:
            first[key] += value
        else:
            first[key] = value
    return first


def subtract_dicts(first, second):
    res = {}
    for key, value in first.items():
        if key not in second:
            res[key] = value
    return res


def parse_email(email_body):
    headers = {}
    content = []
    lines = email_body.splitlines()
    current_header = None
    for line in lines:
        if line == "":
            break
        elif line.startswith((" ", "\t")) and current_header:
            headers[current_header] += " " + line.strip()
        elif ": " in line:
            key, value = line.split(": ", 1)
            headers[key] = value.strip()
            current_header = key
        else:
            continue
    is_body = False
    for line in lines:
        if is_body:
            content.append(line)
        elif line == "":
            is_body = True
    return headers, "\n".join(content).strip()


def extract_links(text):
    links = []
    words = text.split()
    for word in words:
        if word.startswith("http://") or word.startswith("https://"):
            links.append(word)
    return links
