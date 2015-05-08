<h2>Turning Technologies ResponseCard Answer Sniffer</h2>

<b>Required:</b><br>
-Arduino of some sort (I used a Teensy 3.1)<br>
-nRF24L01 transceiver<br>
<br>
<b>What it Does:</b><br>
Receives answers emitted from all nearby clickers on defined channel. Upon receiving an answer it updates a list of all possible answers with their respective count and prints list to serial console.<br>
<br>
<b>To-do:</b><br>
-Transmit answers to master receiver by emulating a clicker (or multiple clickers)<br>
-Make program interactive (set channel at start, reset list for new question, etc.)<br>
-Might look into a red/black tree to replace sequential search to see if it is quicker
<br>
---<br>
<br>
<b>Disclaimer:</b><br>
This should only be used for educational purposes. If you get expelled for academic dishonesty or whatever, that's on you not me.<br>
<br>
<b>Props to:</b><br>
-This guy for reverse engineering clickers and making the information public -  (http://travisgoodspeed.blogspot.com/2010/07/reversing-rf-clicker.html)<br>
-This guy for providing a tl;dr of the above post and providing code to setup the nRF24L01 to communicate with clickers - (http://www.taylorkillian.com/2012/11/turning-point-clicker-emulation-with.html)<br>
<br>
---<br>
<br>
<img src="http://i.imgur.com/TU0spuU.jpg?1" width=640>
<img src="http://i.imgur.com/e9JcLOx.png?1" width=640>
