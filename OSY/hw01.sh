#!/bin/bash

show_help() {
    echo "Reads lines from stdin starting with 'PATH ' and outputs info on filesystem objects."
    echo "---"
    echo "Usage: $0 [-h] [-z]"
    echo "  -h    Prints this help message"
    echo "  -z    Puts files to an .tgz archive"
}

#----------------------

process_line() {
    local files_to_archive=()

    OIFS="$IFS"
    IFS=$'\n'
    while read -r path; do
        if [ -L "$path" ]; then
            local target
            target=$(readlink "$path")
            echo "LINK '$path' '$target'"

        elif [ -d "$path" ]; then
            echo "DIR '$path'"

        elif [ -f "$path" ]; then
            local lines first_line
            if lines=$(wc -l < "$path"); then
                first_line=$(head -n 1 "$path")
                echo "FILE '$path' $lines '$first_line'"
                files_to_archive+=("$path")
            else
                echo "ERROR processing file '$path'" >&2
                exit 2
            fi

        else
            echo "ERROR '$path'" >&2
            exit 1
        fi
    done
    IFS="$OIFS"

    if [ "$create_archive" = 1 ] && [ "${#files_to_archive[@]}" -gt 0 ]; then
        if ! tar czf output.tgz "${files_to_archive[@]}"; then
            echo "ERROR creating archive" >&2
            exit 2
        fi
    fi
}

#----------------------
# Initialize options

create_archive=0
OPTIND=1

while getopts hz opt; do
    case $opt in
        h)
            show_help
            exit 0
            ;;
        z)  
            create_archive=1
            ;;
        *)
            show_help >&2
            exit 2
            ;;
    esac
done

shift "$((OPTIND - 1))"

#----------------------
# Read stdin, filter lines starting with "PATH ", strip prefix, and process

sed '/^PATH /!d;s/^PATH //' | process_line
exit $?