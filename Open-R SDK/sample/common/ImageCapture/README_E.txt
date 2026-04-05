
                                ImageCapture

*Overview

  ImageCapture is a sample program that takes a picture using AIBO's camera.
  This sample is made by combining several other sample programs in the OPEN-R SDK.
  It saves files that consist of raw data from LayerH, LayerM, LayerL, and BMP
  formatted data from LayerH which is converted from raw data.


*Usage

  - After booting, AIBO stands, and ceases movement.

  - When the chin switch or back sensor are pushed, AIBO saves its image data 
    to the /OPEN-R/MW/DATA/P directory.

  - Data file names consist of:
    RGBH0000.BMP - RGBH9999.BMP    LAYER H data converted to BMP format
    LAYH0000.RAW - LAYH9999.RAW    LAYER H raw data
    LAYM0000.RAW - LAYM9999.RAW    LAYER M raw data
    LAYL0000.RAW - LAYL9999.RAW    LAYER L raw data

    The number included in the file name is increased increases with each successive photo.
    If the same file already exists, the file is not over-written.

  - If TinyFTPd is running, you can get the image data over the wireless LAN.
  
  - If you want to shutdown AIBO, push its pause button.
