# roland-capture
I want to be able to change the sample rate on Roland Capture USB 2.0 sound cards from Linux.

To that end I've run Wireshark against the interface while changing its sample rate and captured the packets.

In the focused folder you'll find a nice `csv` summary for each sample rate, a `txt` file with more detail, and the `pcapng` file itself.
None of these include ISOCHRONOUS or INTERRUPT data since that would be a flood.
For the whole enchilada there's the `full` folder.
