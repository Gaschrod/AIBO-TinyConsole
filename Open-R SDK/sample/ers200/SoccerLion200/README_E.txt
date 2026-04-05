
			SoccerLion for ERS-210/220


[Directory]

  SoccerLion200     --+-- OMWares --+-- include  Header file 
                      |             +-- lib      Library 
                      |             +-- MS       MW Object (binary) 
                      |
                      +-- SoccerLion                    
                      |
                      +-- PowerMonitor
                      |
                      +-- MotionCommander   (works only on ERS-210/210A)


[How to build and how to run the SoccerLion sample program]

  1. Build

     $ cd SoccerLion
     $ make install

  2. Copy the following OPEN-R directory to a blank AIBO Programming Memory Stick. 

     /usr/local/OPEN_R_SDK/OPEN_R/MS/WCONSOLE/nomemprot/OPEN-R

     Then, overwrite the Memory Stick with the following OPEN-R directory.

     SoccerLion/MS/OPEN-R

     * SoccerLion works only in the nomemprot enviroment.

  3. Insert the AIBO Programming Memory Stick into AIBO. Then boot AIBO.
     Subsequent operations are the same as other sample programs.

  4. Once AIBO is standing, press the head sensor to activate AIBO. AIBO
     will search for the pink ball.  Once found, AIBO will dribble the ball,
     or kick it.

[How to build and how to run the MotionCommander sample program]

  1. Build

     $ cd MotionCommander
     $ make install

  2. Copy the following OPEN-R directory to a blank AIBO Programming Memory Stick. 

     /usr/local/OPEN_R_SDK/OPEN_R/MS/WCONSOLE/nomemprot/OPEN-R

     Then, overwrite the Memory Stick with the following OPEN-R directory.

     MotionCommander/MS/OPEN-R

     * MotionCommander works only in the nomemprot enviroment.
     * MotionCommander works only on ERS-210/210A.

  3. Insert the AIBO Programming Memory Stick into AIBO. Then boot AIBO.
     AIBO can be remote-controled by the wireless console.

  4. When you get the prompt MotionCommander> in your  wireless console,
     you can input control commands.
     To see the list of the commands, type '?'.
     
  5. To shutdown AIBO, type 'q'.
