#!/bin/sh

ae_logo() {
    GREEN="\e[32m"
    ENDCOLOR="\e[0m"

    echo ""
    echo -e "${GREEN}       .####.   #### .####. ##### ##   ## ##  ##  ${ENDCOLOR}"
    echo -e "${GREEN}       ##  ## ##.    ##  ## ##    ### ### ##  ##  ${ENDCOLOR}"
    echo -e "${GREEN}       ###### '####  ##     ####  ## # ## ##  ##  ${ENDCOLOR}"
    echo -e "${GREEN}       ##  ##    '## ##  ## ##    ##   ## ##  ##  ${ENDCOLOR}"
    echo -e "${GREEN}       ##  ## .org#  '####' ##### ##   ##  ####   ${ENDCOLOR}"
    echo -e "${GREEN}                    A never ending place to work. ${ENDCOLOR}"
}
until [ "${option}" = "x" ]; do
    ae_logo
    echo ""
    echo "  1 - Import Maps."
    echo "  2 - Import Vmaps."
    echo "  3 - Import Mmaps."
    echo ""
    echo "  X - Exit this tool"
    echo ""
    read -p "Enter a char:  " option

    if [ "${option}" = "1" ]; then
        ./map_extractor
    elif [ "${option}" = "2" ]; then
        mkdir -p vmaps
        ./vmap4_extractor
        ./vmap4_assembler Buildings vmaps
    elif [ "${option}" = "3" ]; then
        mkdir -p ./mmaps
        ./mmaps_generator
    fi
    if [ "${option}" != "X" ]; then
        read -p "Press enter to continue..." dummy
    elif [ "${option}" != "x" ]; then
        read -p "Press enter to continue..." dummy
    fi
done
echo "Closing.."
