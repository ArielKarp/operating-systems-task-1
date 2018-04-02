#!/bin/bash

full_name="${FULL_EXE_NAME}"
path_to_data="${PATH_TO_DATA}"
data_file="${DATA_FILE}"
pattern="${PATTERN}"
bound="${BOUND}"

full_data_path="$path_to_data/$data_file"
chmod 700 "$full_data_path"

"$full_name" "$full_data_path" "$pattern" "$bound" &