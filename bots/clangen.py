import os
import json
import re
import sys
import random

special_chars = ['!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '[', ']', '<', '>', '/', '\\', '{', '}', '+', '=', '-', '.', ';', ':', '\'', '"', '`', '~']

# Read the team names from the file
with open('team_names.txt', 'r') as file:
    clan_names = [line.strip() for line in file]

clan_id = 12
file_names = []
clan_infos = {}
clan_name_to_file_name = {}

for clan_name in clan_names:
    words = clan_name.split()
    tag = "".join(word[0] for word in words if word[0].isalpha())
    if len(tag) > 3:
        tag = tag[:3]
    tag = tag.upper()

    # Add a special character at the beginning and/or end of the tag
    special_char_start = special_chars[random.randint(0, len(special_chars) - 1)]
    special_char_end = special_char_start if random.random() < 0.5 else special_chars[random.randint(0, len(special_chars) - 1)]
    tag = special_char_start + tag + special_char_end

    # Ensure the tag is not longer than 5 characters
    if len(tag) > 5:
        tag = tag[:5]

    # Randomize pre_or_post to be 0 or 1
    pre_or_post = random.randint(0, 1)

    # Prepare the clan information
    clan_info = {
        "clan_id": clan_id,
        "clan_tag": tag,
        "clan_name": clan_name,
        "pre_or_post": pre_or_post
    }

    # Prepare the file name
    file_name = re.sub(r'\s+', '', clan_name).lower() + '.json'
    if file_name in file_names:
        print(f"Conflict detected: {file_name} already exists.")
        print(f"Offending Clan Names: {clan_name_to_file_name[file_name]}, {clan_name}")
        sys.exit(1)
    file_names.append(file_name)
    clan_name_to_file_name[file_name] = clan_name

    # Store the clan information
    clan_infos[file_name] = clan_info

    # Increment the clan_id
    clan_id += 1

# Write the clan information to files
for file_name, clan_info in clan_infos.items():
    with open(os.path.join('clans', file_name), 'w') as file:
        json.dump(clan_info, file, indent=4)