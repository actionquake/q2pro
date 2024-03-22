# Action Quake 2: Defend the City
---

## Introduction
This is the main documentation for the Defend the City (DTC) game mode.  It will cover each mode, how to operate a server with this mode, and how to create your own entity files, or edit existing ones.

### What is this?




### Code work to-do
* HUD changes
  * Timer/clock starting from 00:00, incrementing per second
  * Enemy kill counts (total)
  * Lives remaining (player-specific)
* Entity behavior changes
  * Bots do not drop weapons or item (or anything) to keep entity counts low
* Determining spawn point order (front to back)
  * Do this with a custom spawnpoint and custom .ent file, or load custom .dtc file like Espionage/CTF/Dom?
* Removing spawn points as they are 'overrun' by attackers
* Loadouts
  * Weapons
    * Ammo regeneration
      * On intervals
      * On rewards (Impressive, Accuracy, Excellent)
      * On wave completions
      * Ensuring players do not run out of ammo too quickly?
        * Unlimited MK23 Ammo?
  * Items
    * Kevlar Helm and Vest work the same, except they do break after so many hits (20 hits?)
    * Bandolier regenerates ammo 2x faster
    * Silencer ??  Replace with something else?
    * Laser Sight works as normal
    * Slippers work as normal (enhanced by default)
    * Item degredation?
      * Kevlar and Bandolier only lasts for so many hits
      * Silencer and Laser sight only last so many shots?  Or unlimited use
      * Slippers only last so many falls, or unlimited use?
      * cprintf to player when item breaks
* Player respawn count
* Do not respawn player if life count is 0
* How to display life count (Re-use CTF style stats on right side of screen?)
* Increase amount of ammo magazines / bullets that a player can hold at once
* Events (automated events)
  * Enemy wave generation
    * How many?  How often?
  * Rewards on completing events
  * Enemy occupation of area
* Deployments (player initiated events)
  * Reinforcements
  * Automated turrets
  * Traps
  * Explosives
  * Airstrikes
* Enemy AI
  * Goals are, in order of importance:
    * Assault and eliminate any visible player
    * Assault / reach the info_player_dtc points
      * When enemy AI touches the spawnpoint, they capture it
    * Find any remaining players that are not visible and eliminate them
    
