import json
import zlib
import struct
import os

def load_all_clan_data():
    clans = {}
    seen_clan_ids = {}
    for filename in os.listdir('clans'):
        if filename.endswith('.json'):
            with open(f'clans/{filename}', 'r') as file:
                clan = json.load(file)
                if clan['clan_id'] in seen_clan_ids:
                    raise ValueError(f"Duplicate clan_id {clan['clan_id']} found in files {seen_clan_ids[clan['clan_id']]} and {filename}")
                seen_clan_ids[clan['clan_id']] = filename
                clans[clan['clan_id']] = clan

    return clans

def write_bot_to_file(bot, clan, file, index_file, index):
    # Add the clan tag to the bot name if a clan is provided
    if clan:
        if clan['pre_or_post'] == 0:
            bot['name'] = clan['clan_tag'] + bot['name']
        else:
            bot['name'] = bot['name'] + clan['clan_tag']

    if len(bot['name']) > 16:
        raise ValueError(f"Bot name {bot['name']} is too long: {len(bot['name'])} characters (max 16)")
    
    # Convert the bot dictionary to a JSON string
    json_string = json.dumps(bot)

    # Convert the JSON string to bytes
    json_bytes = json_string.encode('utf-8')

    # Compress the bytes
    compressed_bytes = zlib.compress(json_bytes)

    # Get the current position in the file
    offset = file.tell()

    # Write the length of the compressed data
    file.write(struct.pack('I', len(compressed_bytes)))

    # Write the compressed data
    file.write(compressed_bytes)

    # Write the bot's index, name, offset, and length to the index file
    index_file.write(f'{index},{bot["name"]},{offset},{len(compressed_bytes)}\n')

# Load all the clan data
clans = load_all_clan_data()

# Open the binary file and the index file
with open('bots.bin', 'wb') as file, open('index.txt', 'w') as index_file:
    index = 0
    # Load the bot data from multiple JSON files in bots_clan directory
    for bot_filename in os.listdir('bots_clan'):
        if bot_filename.endswith('.json'):
            with open(f'bots_clan/{bot_filename}', 'r') as bot_file:
                bot = json.load(bot_file)

            # Get the clan data for this bot
            clan = clans.get(bot["clan_id"])

            # Write the bot data to the binary file
            write_bot_to_file(bot, clan, file, index_file, index)
            index += 1

    # Load the bot data from multiple JSON files in bots_pub directory
    for bot_filename in os.listdir('bots_pub'):
        if bot_filename.endswith('.json'):
            with open(f'bots_pub/{bot_filename}', 'r') as bot_file:
                bot = json.load(bot_file)

            # Write the bot data to the binary file without a clan
            write_bot_to_file(bot, None, file, index_file, index)
            index += 1