# Jetpack-Joyride-CE
Hey there! Before you keep reading, here are some questions:
1. Do you like games?
2. Do you like math?
3. Do you like playing games instead of doing math like a responsible person?
4. Do you want to become famous as a "Supa-Cool Tech Whiz" at your school?

If you answered "Yes" to any of those, then you should keep reading!

This is a port by King Dub Dub of the popular smartphone game "Jetpack Joyride" by Halfbrick Studios for the TI-Plus CE calculator. It
allows me to relive the glory days of 2014 on my dad's Windows 8 laptop and have a brief glimpse of the times when I was capable of
feeling real emotion... and when I hacked the game for infinite lives and money with a cheat tool. Good stuff!

Software Installation Instructions:

1. Make sure you've installed the TI-Connect software from the Texas Instruments site.
2. Make sure your calculator OS is below 5.5, since the program can't be run on that OS or above due to TI disabling C/Assembly programs.
    - If your OS is 5.5 or higher, use the arTIfiCE exploit tool to run the program: https://yvantt.github.io/arTIfiCE/
    - Curse the name, the one behind it all, Texas Instruments.
3. Send the JETPACK.8xp program and the assorted JPJR appvars from /bin or the prebuilt releases to the calculator.
    - Your calc may start defragging, this is because I optimized for code speed rather than size; it's normal.
4. The files save to the archive by default, and if you put it in the RAM it will disappear when your battery dies or the RAM resets.
5. On the calc, press "prgm" and select JETPACK, or use the CabriJr/arTIfiCE loader per its instructions.
6. Brag to your friends that you're a "l337 h4x0r" with games on your calculator!

Controls:
- Hold [2nd] to rise, release to fall.
- Press [del] to enter the pause menu.
- Press [clear] to exit the game, it'll save for you.

Special thanks to TIny_Hacker for somehow figuring out HalfBrick's proprietary sprite format, without him this would've looked a lot
uglier. Like, REALLY ugly, check the first commit to see what I mean.

NOTES FOR PEOPLE WHO WANT TO BUILD FROM SOURCE:
- The convimg.yaml is written for Mateo's convimg utility version 8.3 and up, it will give an output error if you use a lower version.
- You'll need to run "convimg" in the gfx directory or "make gfx" from the top level since I've stopped adding the converted sprite
files to get GitHub to stop whining about "uPLoAdINg ToO mANy fIlES." Shut up Microsoft.

REMEMBER:
1. I am not to blame for any damage caused to your calc by the program, as it's still in its testing phases.
2. I am not to blame for any damage caused to your corpse/body by your inattention during chemistry; play responsibly when it's safe, not
when you're handling fulminates!
3. I don't claim to own the rights to the original Jetpack Joyride, this is just a bit of fun for the chirren and a learning experience
for me. Most of the assets are not mine, although I have made some inspired by the game to fit various constraints.
4. I created all the code here, although some has been adapted from online sources with edits to fit my usage.
5. This program is NOT for sale, that violates Jetpack Joyride's copyright policy and might result in legal threats from Halfbrick. This
code and the game are free to use by anyone, regardless of shoe size, fruit preference, or calculator color.
6. This project is not to be used for evil except by minors (eighteen and under) and Starbucks.

I hope you enjoy spreading a little madness, and remember never to trust someone with more than four bumper stickers on their car, they're
clearly insane.

Have a nice morning/afternoon/evening!

If you have any suggestions or questions, ask on the original Cemetech post:
https://www.cemetech.net/forum/viewtopic.php?t=16948

If you find any bugs, have any optimization ideas, or anything that may apply, make a fork/issue/request on the repo or forum post. Keep
things public so others are aware of them.
