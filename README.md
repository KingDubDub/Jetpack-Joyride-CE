# Jetpack-Joride-CE
Hey there! Before you keep reading, here's some questions:
1. Do like games?
2. Do you like math?
3. Do you like playing games instead of doing math like a responisble person?

If you answered "Yes" to any of those, then you should keep reading.

This is a port by King Dub Dub of the popular smartphone game "Jetpack Joyride" by Halfbrick Studios, but it's on a TI-Plus CE calculator. It allows me to relive the glory days of 2016 on my dad's Windows 8 laptop... they were simpler times.

Software Installation Instructions:

1. Make sure you've installed the TI-Connect software from the Texas Instruments site.
2. Make sure your calculator OS is below 5.5-5.6, since the program can't be run on those OS's or any that disable C/Assembly programs.
    - If your OS is 5.5 or higher, use the arTIfiCE exploit tool to run the program: https://yvantt.github.io/arTIfiCE/
    - Remember to shake your fist at TI as well.
3. Send the JETPACK.8xp program from /bin to the calculator
4. The progam saves to the archive by default, if you put it in the RAM it will disappear when your battery dies or the RAM resets.
5. On the calc, press "prgm" and select JETPACK.
6. Brag to your freinds that you're a "l337 h4x0r" with games on your calculator!

Controls:
- Hold [2nd] to rise, release to fall.
- Press [del] to enter the pause menu.
- Press [clear] to exit the game.

Special thanks to TIny_Hacker for somehow figuring out HalfBrick's proprietary sprite format, without him this would've looked a lot uglier. Like, REALLY ugly, check the first commit to see what I mean.

NOTES FOR PEOPLE WHO WANT TO BUILD FROM SOURCE:
- The convimg.yaml is written for Mateo's convimg utility version 8.3 and up, it will give an output error if you use a lower version.
- You'll need to run "convimg" in the gfx directory or "make gfx" from the top level since I've stopped adding the converted sprite
files to get GitHub to stop whining.

REMEMBER:
1. I am not to blame for any damage caused to your calc by the program, as it's still in its testing phases.
2. I am not to blame for any damage caused to your corpse/body by your inattention during chemisty; play responsibly when it's safe, not when you're handling fulminates!
3. I don't claim to own the rights to the original Jetpack Joyride, this is just a bit of fun for the chirren and a learning experience for me.
4. This program is NOT for sale, that violates the license and Jetpack Joyride's copyright policy. This code and the game are free to use by anyone, regardless of shoe size or fruit preference.

I hope you enjoy spreading a little madness, and remember never to trust someone with more than four bumper stickers on their car, they're clearly insane.

Have a nice mid-to-late afternoon!

If you have any suggestions or questions, ask on the original Cemetech post:
https://www.cemetech.net/forum/viewtopic.php?t=16948

If you find any bugs, have any optimization ideas, or anything that may apply, make a fork/issue/request on the repo or forum post.
