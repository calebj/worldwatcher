#World Watcher II
Authors:
* Caleb Johnson-Tasks 1,2,3,4,5,6,Extra
* Alex Kaiser-Tasks 1,2,4
* Jae Jun-Tasks 1,2,5,Extra
* Each member did a fair share of work.

Problem:

You are a secret agent attempting to calculate best positioning of satellites to survey the planet in its entirety with the most efficiency. In order to do this the secret agent must use his knowledge of spherical coordinates and general judgement to place the satellites around a mercator projection of earth. This project's purpose is to create a simple game using the C++ FLTK and GUI functionalities as well as the robust standard library of C++ commands.

Restrictions:

Restrictions include the fact that the secret agent only has access to 2-8 satellites and must position them accurately to survey the most possible land around sperical earth. The agent must place the multiple satellites on a mercator map as far apart as possible, with each satellite having a certain amount of fuel that slowly deminishes the amount of space each satellite can cover with each move. The programmers of the game are also not allowed to have a function longer than 24 lines in order to streamline any debugging necessary to make the game perform at its maximum.

Approach:

We began by using a service called GIT, which easily combines the coding of each individiual member into one master code. We created a master .cpp file titled worldwatcher which contained the master code for the window and multiple attatchments for the game. The FLTK files were modified slightly in order to accomidate certain aspects such as attatching a button over an image and PNG support.

Sample Run:

Results and Analysis:

The project was successful. All basic aspects of the project were covered as well as the extra items. The high scores menu contains the top 5 scores, the game works in all difficutlties and the spehrical coordinates/wraparound are incredibly accurate. The game clock properly concatinates to the difficulty level and the score multiplier is also on point. The hint button shows the user the best move to make due by shading the satellite that should be moved next to obtain a higher score. 

Conclusion:

This program shows that the FLTK/GUI aspects of C++ are capable of creating and running a simple game. We learned multiple C++ tactics such as subclassing, attatching objects and basic GUI controls. On top of this, we learned the powerful aspects of the FLTK program such including the many callbacks and robust functions it has to offer. As an introductory course to C++, this project was a good test of the power of C++ while learning the foundations of programming.

Instructions:

Complile the code using the makefile included in the CD/folder and run using ./worldwatcher when in the worldwatcher directory. The make file should contain the modified FLTK and GUI files as well as the worldwatcher.cpp file, if the code refuses to compile check the makefile to see if this holds true. All instructions on how to work the game itself are included in the "How to play" section.

Program Code:

Bibliography:

No references used.




