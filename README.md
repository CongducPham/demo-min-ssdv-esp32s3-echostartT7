Demonstrating serial data transfer between an ESP32S3 and the EchoStar Terminal T7
=======================================================

With Pr. Fabien Ferrero from LEAT laboratory, we are collaborating on satellite transmission of IoT data. He and his team are developing dedicated cost-effective antennas and terminal to send IoT data to LEO satellite. Their latest development board is the [EchoStar Terminal T7](https://github.com/nguyenmanhthao996tn/LEAT-EchoStar-Terminal-BSP) which uses an EchoStar EM2050 radio to send to the EchoStar satellite.

In the context of this collaboration, we tested serial data transfer between an ESP32S3 and the EchoStar Terminal T7. The idea is to use the XIAO ESP32S3-Sense (see our[LoRaCAM-AI](https://github.com/CongducPham/PEPR_AgriFutur/tree/main/Arduino_ESP32)) to take pictures, process the picture and eventually transmit the picture. In case of picture transmission, the picture will be first encoded to reduce its size and increase its robusteness against packet losses.

<img src="https://github.com/CongducPham/demo-min-ssdv-esp32s3-echostartT7/blob/main/images/esp32s3_echostarT7.jpg" width="500">

We developed a proof-of-concept using MIN ([Microcontroller Interconnect Network](https://github.com/min-protocol/min)) to implement the serial data transfer and SSDV ([Slow Scan Digital Video](https://ukhas.org.uk/doku.php?id=guides:ssdv)) to encode the picture. The [SSDV library](https://github.com/fsphil/ssdv) is provided by Philip Heron.

During this process, we learned a lot of things on MIN and SSDV and this repository both acknowledges the great work from MIN & SSDV and also wanted to provide an operational example that can help others to overcome some implementation issues when dealing with MIN & SSDV.

Basically, there is a transmitter, which is the XIAO ESP32S3-Sense, that encodes a statically stored YCbCr jpeg image using SSDV and transmits each SSDV packet to the EchoStar-T7, acting as receiver of SSDV packets. In the example, there is no still transmission using the EchoStar EM2050 radio but it will be straightforward to do so as the  EM2050 radio can be driven by simple AT commands.

We also provide in the `tools` folder some scripts for image manipulation: "normal" jpeg to YCrCb jpeg, then SSDV encoding, then SSDV decoding, then back to "normal" jpeg to visualize the SSDV decoded image. The general process is:

- start with a "normal" jpeg image, assuming it is person240x240.jpg
- then, convert to YCbCr format, here with use the OpenCV Python library to do so
  > python convert_to_ycbcr.py person240x240
- then, we encode using SSDV  
  > ./ssdv -e -c ECHOT7 -i 0 -q 4 -l 80 person240x240_yCbCr.jpg person240x240_yCbCr_80_q4.bin
- then, we decode right away
  > ./ssdv -d -l 80 person240x240_yCbCr_80_q4.bin output_person240x240_80_q4.jpg
- finally, we convert the YCbCr back to "normal" jpeg for visualization  
  > python convert_back_to_sRGB.py output_person240x240_80_q4
- this last step will produce `output_person240x240_80_q4_restored_srgb.jpg`

You can achieve the same result by using the `ssdv_tool_chain.sh` script:

  > ssdv_tool_chain.sh person240x240 4 80

Where 4 is the SSDV quality and 80 is the SSDV packet size. Note that you should not put the `.jpg` extension for the `ssdv_tool_chain.sh` script.

Enjoy!
C. Pham
