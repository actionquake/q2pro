# Initialize lists
names_longer_than_16 = []
names_exactly_16 = []
names_less_than_10 = []

import json
import random
import os
import re

# Function to generate a random value with a bias towards 0.5, 1 decimal place
def generate_random_value():
    return round(random.normalvariate(0.5, 0.2), 1)

with open('skin_names.txt', 'r') as file:
    skin_names = [line.strip() for line in file]

# Open and read the file
with open('bot_names.txt', 'r') as file:
    for line in file:
        name = line.strip()  # Remove newline characters and whitespace
        length = len(name)
        
        # Check the length and create a bot profile if it's 10 characters or less
        if length <= 10:
            # Randomly select whether the bot should be added to a clan
            add_to_clan = random.random() > 0.5

            bot_profile = {
                "name": name,
                "clan_id": random.randint(1, 110) if add_to_clan else None,
                "skin": random.choice(skin_names),  # Randomly select a skin
                "weaponPreferences": {key: generate_random_value() for key in ["mk23", "dual_mk23", "mp5", "m4", "m3", "hc", "sniper", "knives", "grenades"]},
                "itemPreferences": {key: generate_random_value() for key in ["vest", "helm", "laser", "silencer", "slippers", "bandolier"]},
                "coreTraits": {key: generate_random_value() for key in ["aggressiveness", "teamwork", "curiosity", "aimingSkill", "reactionTime", "communicationFreq", "communicationTone", "movementStyle", "objectiveFocus"]}
            }

            # Remove special characters from the filename
            filename = re.sub('[^a-zA-Z0-9_]', '', name.lower().replace(' ', ''))

            # Write the bot profile to a new JSON file in the appropriate directory
            directory = 'bots_clan' if add_to_clan else 'bots_pub'
            filename = os.path.join(directory, filename + '.json')
            with open(filename, 'w') as outfile:
                json.dump(bot_profile, outfile, indent=4)
        
        # Check the length and create a bot profile if it's exactly 16 characters
        # At 16 characters, the bot will not be added to a clan because they can't add a clan tag to their name
        if length == 16:
            bot_profile = {
                "name": name,
                "clan_id": None,
                "skin": random.choice(skin_names),  # Randomly select a skin
                "weaponPreferences": {key: generate_random_value() for key in ["mk23", "dual_mk23", "mp5", "m4", "m3", "hc", "sniper", "knives", "grenades"]},
                "itemPreferences": {key: generate_random_value() for key in ["vest", "helm", "laser", "silencer", "slippers", "bandolier"]},
                "coreTraits": {key: generate_random_value() for key in ["aggressiveness", "teamwork", "curiosity", "aimingSkill", "reactionTime", "communicationFreq", "communicationTone", "movementStyle", "objectiveFocus"]}
            }

            # Remove special characters from the filename
            filename = re.sub('[^a-zA-Z0-9_]', '', name.lower().replace(' ', ''))

            # Write the bot profile to a new JSON file
            filename = os.path.join('bots_pub', filename + '.json')
            with open(filename, 'w') as outfile:
                json.dump(bot_profile, outfile, indent=4)

        # Check the length and add to the appropriate list
        if length > 16:
            names_longer_than_16.append(name)

    if len(names_exactly_16) > 0:
        print("Excluded bot names due to 16 character limit:", names_exactly_16)
