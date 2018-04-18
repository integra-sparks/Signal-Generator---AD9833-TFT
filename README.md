# Signal-Generator---AD9833-TFT
Using various sources, a signal generator up to 4MHz 
has been cobbled together from low cost components. Basic soldering skills are required to create a reliable back pack for the TFT
The TFT was purchased from bitsbox.co.uk and is a touch screen. Other Touch screens can be 
used or you can of course use buttons. The touch screen makes for a nice clean build and is easy to prototype. 
The AD9833 cards vary a bit, the one used in this project has two 50ohm outputs one can go to more or 
less full rail. I used a 5v regulator on this but the whole unit can work of 3.3v quite satisfactorily. 
The project started with an Arduino UNO however I wanted a more compact unit so was going to use a larger nano, 
but instead used a MASSDUINO Core. This makes for a more compact unit. A smaller footprint is still possible 
either with a PCB for the backpack board or just pushing things in a bit more. this was not really required in this case but gives room for tweaking your design. The libraries are all available from Github, So a quick BOM
1: Uno, MASSDUINO Core or other larger Arduino or clone (168's are too small)
2: 9833 board, look for the one with two outpous
3: 3.3V regulator board (or use a regulator - the screen is a bit too much for the on board supply)
4: 5V regulator (optional)
5: Power supply and connectors to suit
6: Proto board PCB
7: relevant connectors

note: 5v regulator used was a 7805, a .47uf electrolytic was used across the input (pin 1 Vin and pin 2 ground, the negative side of the cap going to ground). 
