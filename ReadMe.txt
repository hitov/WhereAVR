For customizing this code for your own project, note that the files you
will need to change are the Main.c, Messaging.c, and  main_eeprom.bak files.

Main.c contains gobs of custom code to decode commands, drive servos,
and mainly give the tracker its personality.  I tried to comment as much as
possible, and you will find much of the code (like where I drive a servo)
can be ignored or deleted.  I left it in as a guideline to see where to
add your own hacks.  You probably won't want to touch anything outside of
the main(void) function, as most everything else is optimized for a specific
task.

Messaging.c contains the functions which create the individual messages you
want to transmit.  I have included the position and telemetry messages I use
on my flights.  Again, feel free to hack, as it should be obvious how to add
and delete characters and fields.

Finally, main_eeprom.bak is where you will store your path and callsign info.
Detailed instructions on how to modify this file are located in the ax25.c 
file.  For modifying the file, I recommend the Hexplorer program available at
http://artemis.wszib.edu.pl/~mdudek/

When programming, keep in mind that setting the "Preserve EEPROM memory
through Chip Erase cycle" fuse will keep you from having to reprogram
the EEPROM every time you flash the chip's program space.  This is very 
convenient, and saves lots of time.

Finally, please keep in mind that when a packet is transmitted, the serial
interrupts are disabled.  This produces the most pure, decodable tone.
However, it also opens the opportunity for lost incomming characters.  This
may result in a single clobbered GPS report.  So, for one second after each
transmission, GPS data is not valid.  Wait a second, and everything will sync
right back up.

If you have questions or comments (such as a suggestion on how to create
the eeprom file using make <- I'm sure it's possible, but I haven't done it
yet) please feel free to e-mail me at me@garydion.com.