#!/bin/bash

show_help() {
    echo "Performs various text operations depending on the selected option."
    echo "---"
    echo "Usage: $0 [-h] [-a] [-b] [-c] [-d PREFIX]"
    echo "  -h    Prints this help message"
    echo "  -a    Prints all PDF files from the current directory (case-insensitive, sorted)"
    echo "  -b    Reads text from stdin and outputs all lines starting with a number, without the number"
    echo "  -c    Reads text from stdin and outputs all sentences, one per line"
    echo "  -d    Reads C source code from stdin and adds PREFIX to filenames in all #include directives"
}

#----------------------

print_pdf() {
    find . -type f -iname "*.pdf" -printf '%f\n' | sort
}

#----------------------

print_numbered_lines() {
    sed -E '/^[+-]?[0-9]+/!d;s/^[+-]?[0-9]+//'
}

#----------------------

print_sentences() {
   tr -d '\r' | tr '\n' ' ' | grep -oE '[[:space:]]*[^.!?]*[.!?]' | sed 's/^[[:space:]]*//'
}

#----------------------

add_prefix() {
    PREFIX=$1
    sed -E "s@(#[[:space:]]*include[[:space:]]*)<([^>]+)>@\1<${PREFIX}\2>@g; \
            s@(#[[:space:]]*include[[:space:]]*)\"([^\"]+)\"@\1\"${PREFIX}\2\"@g"
}

#----------------------
# Run function based on the options

OPTIND=1

while getopts habcd: opt; do
    case $opt in
        h)
            show_help
            exit 0
            ;;
        a)  
            print_pdf
            exit 0
            ;;
        b)  
            print_numbered_lines
            exit 0
            ;;
        c)  
            print_sentences
            exit 0
            ;;
        d)  
            add_prefix "$OPTARG"
            exit 0
            ;;
        *)
            show_help >&2
            exit 1
            ;;
    esac
done

shift "$((OPTIND - 1))"