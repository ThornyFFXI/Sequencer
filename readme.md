# Sequencer - TLDR
This stops some extra server hassle and prevents some double messages ('Unable to cast spells at this time.', 'You must wait longer to use that ability.', etc).  It is safe for use with all versions of Ashitacast, LuAshitacast, and LegacyAC on Ashita4.  It is recommended that you use it if you use any of these systems, even if you do not understand what it's doing.<br>

# Installation
Do not download the entire repository as a zip unless you plan to compile the plugin yourself. That will not provide the files you need.<br>

1. Download the plugin release zip that matches your ashita installation's interface version from the releases area on the right side.
2. Extract the entire zip to your ashita folder(the folder containing ashita.dll and ashita-cli.exe). Everything will fall into place.
3. Load with '/load sequencer' or add the same line to your startup script. Sequencer is a plugin not an addon, so do not use '/addon load'.

## The Problem
Every packet/instruction sent from the FFXI client comes with a sequence ID, to indicate whether it's been sent already.  Because users cannot be expected to track these at all times, Ashita automatically patches all sequence IDs of injected packets to match the first packet in the chunk.  This is safe and guarantees nobody will desync with the server.  But, when the client has to resend an existing packet, it will have a lower sequence ID than the first packet.  This means that in laggy situations, if your actions do not arrive in time, they get resent.  Because AC/LAC need to block the real packet and inject a new packet to get your swaps in before it, sequence IDs will all be modified by Ashita.  This causes the resend to evaluate independantly on the server, and count as a second copy of the same action, instead of having the server rightfully realize that it was already sent and processed.<br><br>

## The Effects
In older versions of AC and LAC, during laggy periods, actions would be processed twice by the server.  This means you might start casting a spell and see both the 'Yourname starts casting xxx' and the 'Unable to cast spells at this time.' message.  This is a minor inconvenience, but it's also extra unnecessary load on the server, at a time when it's likely already struggling.  Newer versions will simply block resent packets, but that means your action may be dropped entirely when your connection is unstable, rather than benefitting from the game's automatic retry.<br><br>

## The Solution
This plugin checks outgoing data before and after Ashita modifies it.  By taking the sequence ID of an action packet prior to Ashita's modifications, and applying it to all actions and equip changes after Ashita's modifications, we can ensure that any resent packet is recognized as a resend packet.  Ashitacast 1.20+, LuAshitacast 2.00+, and LegacyAC 1.80+ will automatically check if Sequencer is loaded, and only allow resent packets through if it is.  This means that users of Sequencer will get the full built-in retry capability of the game with no side effects, while those who do not have Sequencer will not get retries but also not get double messages.  While this may seem like a minor issue, there is no downside to using Sequencer and it is beneficial to yourself and the server you play on.  I highly recommend anyone using any of these swaps systems to keep it loaded at all times.