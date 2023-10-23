# GreekBot
**GreekBot** is a discord bot client made from scratch, intended to be the main moderation and support bot for the **Learning Greek** discord server, a large community of learners of the greek language from all over the world. You can check it out at https://discord.gg/greek

# Design
The code base is split into 2 distinct parts:
1. ## Foundation
   The foundation is an interface which implements all the low-level functionality for interacting with the Discord API. As it is, it can be forked and used as a base library for other Discord bots as well!
3. ## Implementation
   The actual implementation of all the server-specific features for **Learning Greek**

# Features
**GreekBot** supports the following commands:
- `/avatar` which responds with a user's current profile picture
- `/prune ` which initiates member pruning with additional parameters than the stock discord client offers. For **Learning Greek** specifically, this command kicks members who fail to complete our verification process within 48 hours.
- `/ban   ` which bans specified users, while also informing them of the reason for the ban in their private messages.

Additionally, these features are also implemented
- An XP system based on message activity, just like MEE6's (not *fully* complete yet)
- A custom role selection system with interactive buttons and dropdown menus

# Planned features
- A starboard system, "pinning" popular messages based on certain reactions
- More moderation features, like a warning system or time-limited bans
