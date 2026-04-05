
[Building & Running W3AIBO]

1. Prepare libjpeg.

   You need to download jpegsrc.v6b.tar.gz.

   $ ls
   MS        README_E.txt  W3AIBO              makefile.aibo
   Makefile  README_J.txt  jpegsrc.v6b.tar.gz   

2. Build the executable files.

   $ make install

3. First, copy the following OPEN-R directory to a blank AIBO
   Programming Memory Stick.
   
     /usr/local/OPEN_R_SDK/OPEN_R/MS/WCONSOLE/nomemprot/OPEN-R

     Then, overwrite the Memory Stick with the following OPEN-R directory.

     MS/OPEN-R
     
4. Insert the AIBO Programming Memory Stick into AIBO. Then boot AIBO.
   Subsequent operations (usage of wireless console, how to shutdown
   AIBO, etc.) are the same as before.

5. Access AIBO with web browser.

   Layer H image
     http://<AIBO's IP address>:60080/        
     http://<AIBO's IP address>:60080/layerh
     http://<AIBO's IP address>:60080/layerhr (Reconstructed image)

   Layer M image
     http://<AIBO's IP address>:60080/layerm  
     http://<AIBO's IP address>:60080/layermr (Reconstructed image)

   Layer L image
     http://<AIBO's IP address>:60080/layerl
     http://<AIBO's IP address>:60080/layerlr (Reconstructed image)
