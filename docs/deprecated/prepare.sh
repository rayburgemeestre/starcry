typeset instruction_file=
mkdir build_instructions 2>/dev/null
cat ../prepare_linux.sh | while IFS= read -r line; do
    if [[ $line =~ \#END ]]; then
        instruction_file=
    fi
    if [[ $instruction_file != "" ]]; then
        echo "$line" >> $instruction_file
    fi
    if [[ $line =~ \#BEGIN: ]]; then
        typeset build=${line//#BEGIN: /}
        instruction_file=build_instructions/$build
        rm -rf $instruction_file 2>/dev/null
        touch $instruction_file
    fi
done

../starcry --help > cli_help.txt
make clean
make html
