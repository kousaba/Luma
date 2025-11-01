#!/bin/bash

# Change to the source directory
cd "$1"

# Extract ANTLR options and grammar file
ANTLR_JAR="$2"
OUTPUT_DIR="$3"
GRAMMAR_FILE="${@: -1}" # Last argument is the grammar file
ANTLR_OPTIONS="${@:4:$#-4}" # Arguments from 4 to (last-1) are ANTLR options

# Execute the ANTLR tool
echo "DEBUG: Java command: java -jar \"${ANTLR_JAR}\" -Dlanguage=Cpp -o \"${OUTPUT_DIR}\" -visitor ${ANTLR_OPTIONS} \"${GRAMMAR_FILE}\""
java -jar "${ANTLR_JAR}" -Dlanguage=Cpp -o "${OUTPUT_DIR}" -visitor ${ANTLR_OPTIONS} "${GRAMMAR_FILE}"