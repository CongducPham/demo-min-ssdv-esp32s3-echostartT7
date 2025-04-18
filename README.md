Demonstrating serial data transfer between an ESP32S3 and the EchoStar Terminal T7
=======================================================

With Pr. Fabien Ferrero from LEAT laboratory, we are collaborating on satellite transmission of IoT data. He and his team are developing dedicated cost-effective antennas and terminal to send IoT data to LEO satellite. Their latest development board is the [EchoStar Terminal T7](https://github.com/nguyenmanhthao996tn/LEAT-EchoStar-Terminal-BSP) which uses an EchoStar EM2050 radio to send to the EchoStar satellite.

In the context of this collaboration, we tested serial data transfer between an ESP32S3 and the EchoStar Terminal T7. The idea is to use the XIAO ESP32S3-Sense (see our[LoRaCAM-AI](https://github.com/CongducPham/PEPR_AgriFutur/tree/main/Arduino_ESP32)) to take pictures, process the picture and eventually transmit the picture. In case of picture transmission, the picture will be first encoded to reduce its size and increase its robusteness against packet losses.

<img src="https://github.com/CongducPham/demo-min-ssdv-esp32s3-echostartT7/blob/main/images/esp32s3_echostarT7.jpg" width="500">

We developed a proof-of-concept using MIN ([Microcontroller Interconnect Network](https://github.com/min-protocol/min)) to implement the serial data transfer and SSDV ([Slow Scan Digital Video](https://ukhas.org.uk/doku.php?id=guides:ssdv)) to encode the picture. The [SSDV library](https://github.com/fsphil/ssdv) is provided by Philip Heron.

During this process, we learned a lot of things on MIN and SSDV and this repository both acknowledges the great work from MIN & SSDV and also wanted to provide an operational example that can help others to overcome some implementation issues when dealing with MIN & SSDV.

Basically, there is a transmitter, which is the XIAO ESP32S3-Sense, that encodes a statically stored YCbCr jpeg 240x240 image (`person240x240_YCrCb_image_array.h`) using SSDV and transmits each SSDV packet to the EchoStar-T7, acting as receiver of SSDV packets. In the example, there is still no transmission using the EchoStar EM2050 radio but it will be straightforward to do so as the EM2050 radio can be driven by simple AT commands. Connect the ESP32S3 to EchoT7 as follows:

```
  ESP32S3     EchoT7
  D7 (RX)       PA2
  D6 (TX)       PA3

```

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

Here is an example of the output. First, from the XIAO ESP32S3 transmitter that encodes the statically stored YCbCr jpeg 240x240 image (`person240x240_YCrCb_image_array.h`) using SSDV. You can actually see that the generated packets are the same from those stored in the `person240x240_yCbCr_80_q4.bin` file.

```
Using Microcontroller Interconnect Network (MIN)
with T_MIN transport protocol
Starting...
send_image_ssdv()
--> Sending SSDV packet #0 to EchoT7...
556635F8EDD20000000F0F000000009697A0A4A0F4AEA6ECAE6537A0828A28AE7200F4A777A6D2F7FC2AE1BB307AB1D1CE8510A88EC87D4937001A9B648B701CCFF790D37A615914DEE9503FCCFD7863
with T_MIN
--> Sending SSDV packet #1 to EchoT7...
556635F8EDD20000010F0F00020003133F9A8A28AEB39EC14514503B0546FD69E7348719E959D48DD58696A02B5023AACA3D311E9BF9B60814D46590A5E8E5FCEE9101F42248445B23287F754332DEAA
with T_MIN
--> Sending SSDV packet #2 to EchoT7...
556635F8EDD20000020F0F0017000708CF6A5EB5111F390063A54B9FF3574E6E77E645C9687F980E293A103B94DCD7D520E7415936925D13B825F2965367A299752E0D382A3D249F289192B5BA38E687
with T_MIN
--> Sending SSDV packet #3 to EchoT7...
556635F8EDD20000030F0F000F0008D29E94D660AC2BB2B34A29FA18C51F9875A45209247E34858152030E69CD9B5063A8A7B2447B01A50C09D1437E71F860F40730403A84C83E88566C6C68C834E076
with T_MIN
…
```

Then, from the EchoT7 terminal that receives the SSDV packets.

```
Using Microcontroller Interconnect Network (MIN)
with T_MIN transport protocol
Starting...
MIN frame with ID 1 received at 24725
ESP32S3-BEGIN-SSDV-i0-q4-l80

MIN frame with ID 2 received at 24748
SSDV packet, skip ASCII print
>556635F8EDD20000000F0F000000009697A0A4A0F4AEA6ECAE6537A0828A28AE7200F4A777A6D2F7FC2AE1BB307AB1D1CE8510A88EC87D4937001A9B648B701CCFF790D37A615914DEE9503FCCFD7863
MIN frame with ID 2 received at 29771
SSDV packet, skip ASCII print
>556635F8EDD20000010F0F00020003133F9A8A28AEB39EC14514503B0546FD69E7348719E959D48DD58696A02B5023AACA3D311E9BF9B60814D46590A5E8E5FCEE9101F42248445B23287F754332DEAA
MIN frame with ID 2 received at 34796
SSDV packet, skip ASCII print
>556635F8EDD20000020F0F0017000708CF6A5EB5111F390063A54B9FF3574E6E77E645C9687F980E293A103B94DCD7D520E7415936925D13B825F2965367A299752E0D382A3D249F289192B5BA38E687
MIN frame with ID 2 received at 39820
SSDV packet, skip ASCII print
>556635F8EDD20000030F0F000F0008D29E94D660AC2BB2B34A29FA18C51F9875A45209247E34858152030E69CD9B5063A8A7B2447B01A50C09D1437E71F860F40730403A84C83E88566C6C68C834E076

```

We actually defined 3 channel for MIN communication:

```
#define CTRL_CHANNEL		      0
#define INFO_CHANNEL        	1
#define SSDV_DATA_CHANNEL   	2
```

Enjoy!
C. Pham
