//#define Serial_print(param)     Serial.print(param)
//#define Serial_println(param)   Serial.println(param)
//#define Serial_printf(fmt, ...) Serial.printf(fmt, ##__VA_ARGS__)

#define RX_PIN          D7
#define TX_PIN          D6
#define ECHOT7          Serial1
#define ECHOT7_BAUD     115200
#define SERIAL_BUF_SIZE 255   // max size of the serial packet

#define CTRL_CHANNEL        0
#define INFO_CHANNEL        1
#define SSDV_DATA_CHANNEL   2

#define USE_MIN
//#define ENABLE_MIN_DEBUG
//#define MIN_SHOW_DEBUG_1
//#define MIN_SHOW_DEBUG_2
#define MIN_SHOW_HEXSTRING

//#define USE_TEST_SSDV_PKT
#define USE_TEST_JPG_YCRCB_ARRAY_TO_SSDV
#define DISPLAY_SSDV_PACKETS

#ifdef USE_TEST_SSDV_PKT
// this is the first packet of person240x240_yCbCr_100.bin which has been generated from person240x240_yCbCr.jpg
// where packet size is set to 100 bytes
// > ./ssdv -e -l 100 person240x240_yCbCr.jpg person240x240_yCbCr_100.bin
// > ./ssdv -e -c ECHOT7 -i 0 -q 4 -l 100 person240x240_yCbCr.jpg person240x240_yCbCr_100.bin
const uint8_t example_ssdv_packet[] = {
0x55, 0x66, 0x35, 0xF8, 0xED, 0xD2, 0x00, 0x00, 0x00, 0x0F, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x96, 0x97, 0xA0, 0xA4, 0xA0, 
0xF4, 0xAE, 0xA6, 0xEC, 0xAE, 0x65, 0x37, 0xA0, 0x82, 0x8A, 0x28, 0xAE, 0x72, 0x00, 0xF4, 0xA7, 0x77, 0xA6, 0xD2, 0xF7, 
0xFC, 0x2A, 0xE1, 0xBB, 0x13, 0x16, 0x8A, 0x28, 0xAD, 0x87, 0x60, 0xA2, 0x8A, 0x28, 0x1D, 0x82, 0xA3, 0x7E, 0xB4, 0xF3, 
0x9A, 0x43, 0x8C, 0xF4, 0x24, 0xA1, 0xDB, 0x81, 0x1A, 0xBC, 0xCA, 0xDA, 0x41, 0x73, 0xAC, 0x40, 0x3C, 0xB3, 0x03, 0xCC, 
0xB5, 0x8D, 0x2C, 0x9F, 0x08, 0x55, 0xAF, 0xC2, 0x5C, 0xE3, 0x70, 0x36, 0x35, 0xAA, 0x07, 0x10, 0x85, 0x30, 0x30, 0x37
}; 
#endif

#ifdef USE_TEST_JPG_YCRCB_ARRAY_TO_SSDV

#define IMG_CHUNK_SIZE_FOR_SSDV   128   // size of the buffer feeding SSDV process 

// Include the array
#include "person240x240_YCrCb_image_array.h"
#include "ssdv.h"
#include "rs8.h"

// 0-7 corresponding to JPEG quality: 13, 18, 29, 43, 50, 71, 86 and 100
uint8_t MY_SSDV_quality = 4;
// will be increased at each image
uint8_t MY_SSDV_image_id = 0;
// the target size for generated SSDV packets
uint8_t MY_SSDV_packet_length = 80; 
// we do not expect to send more than 200 pkts per SSDV image
uint8_t MY_SSDV_max_packet_num = 200;

char MY_SSDV_CALLSIGN[] = "ECHOT7";

ssdv_t ssdv;
#endif

#ifdef USE_MIN

#include "min.h"

// A MIN context (we only have one because we're going to use a single port).
// MIN 2.0 supports multiple contexts, each on a separate port, but in this example
// we will use just SerialUSB.
struct min_context min_ctx;

void min_debug_print(const char *msg, ...) {
#ifdef ENABLE_MIN_DEBUG  
  char debug_print_buf[200];

  va_list arglist;
  va_start(arglist, msg);
  vsprintf(debug_print_buf, msg, arglist);
  va_end(arglist);
  Serial.print(debug_print_buf);
#endif  
}

////////////////////////////////// CALLBACKS ///////////////////////////////////

// Tell MIN how much space there is to write to the serial port. This is used
// inside MIN to decide whether to bother sending a frame or not.
uint16_t min_tx_space(uint8_t port)
{
  // Ignore 'port' because we have just one context. But in a bigger application
  // with multiple ports we could make an array indexed by port to select the serial
  // port we need to use.
  uint16_t n = ECHOT7.availableForWrite();

  return n;
}

// Send a character on the designated port.
// Here we send using ECHOT7 = Serial1 which communicates with the EchoT7
void min_tx_byte(uint8_t port, uint8_t byte)
{
  // Ignore 'port' because we have just one context.
  ECHOT7.write(&byte, 1U);  
}

// Tell MIN the current time in milliseconds.
uint32_t min_time_ms(void)
{
  return millis();
}

void min_tx_start(uint8_t port) {
#ifdef MIN_SHOW_DEBUG_2  
#ifdef TRANSPORT_PROTOCOL
  Serial.print("T_MIN TX start\n");
#else
  Serial.print("MIN TX start\n");
#endif  
#endif
}

void min_tx_finished(uint8_t port) {
#ifdef MIN_SHOW_DEBUG_2  
#ifdef TRANSPORT_PROTOCOL
  Serial.print("T_MIN TX finished\n");
#else
  Serial.print("MIN TX finished\n");
#endif
#endif
}

// Handle the reception of a MIN frame. This is the main interface to MIN for receiving
// frames. It's called whenever a valid frame has been received (for transport layer frames
// duplicates will have been eliminated).
void min_application_handler(uint8_t min_id, uint8_t const *min_payload, uint8_t len_payload, uint8_t port)
{
  // In this simple example application we just echo the frame back when we get one, with the MIN ID
  // one more than the incoming frame.
  //
  // We ignore the port because we have one context, but we could use it to index an array of
  // contexts in a bigger application.
  Serial.printf("MIN frame with ID %d received at %lu\n", min_id, min_time_ms());

  for (uint8_t i=0; i< len_payload; i++) {
    Serial.print((char)min_payload[i]);
  }

  Serial.print("\n");

#ifdef MIN_SHOW_HEXSTRING
  for (uint8_t i=0; i< len_payload; i++) {
    //if (min_payload[i] < 0x10)
    //  Serial_print("0");
    Serial.printf("%.2X", min_payload[i]);
  }
#endif

  Serial.print("\n");
}
//////////////////////////////// END CALLBACKS /////////////////////////////////

void local_min_poll() {
  uint8_t c;
  size_t recv_buf_len=0;

  // to get data from the EchoT7 such as ACK
  if (ECHOT7.available()) {
    c = ECHOT7.read();
    recv_buf_len = 1;  
  }
  else {
    recv_buf_len = 0;
  }  
  // .. and push them into MIN. It doesn't matter if the bytes are read in one by
  // one or in a chunk (other than for efficiency) so this can match the way in which
  // serial handling is done (e.g. in some systems the serial port hardware register could
  // be polled and a byte pushed into MIN as it arrives).
  min_poll(&min_ctx, &c, (uint8_t)recv_buf_len); 
}
#endif // USE_MIN

uint32_t my_get_time_ms()
{
  return millis();
}

// Read from camera buffer or image buffer
size_t fill_ssdv_buffer(uint8_t *buffer, size_t chunk_size, uint8_t *image_buffer, size_t image_buffer_len, size_t i_index) {
  size_t buf_size = 0;

  if ((i_index + chunk_size) < image_buffer_len) {
    buf_size = chunk_size;
  }  else  {
    buf_size = image_buffer_len - i_index;
  }
  // clear the dest buffer
  memset(buffer, 0, chunk_size);
  memcpy(buffer, image_buffer+i_index, buf_size);
  return buf_size;
}

int send_image_ssdv(uint8_t* source_image_buf, size_t source_image_size) {

  char print_buf[SERIAL_BUF_SIZE];
  uint8_t print_buf_len;
  uint32_t ssdv_last_sent=0;
  size_t i_index = 0;
  uint8_t MY_SSDV_packet_count = 0;
  uint8_t MY_SSDV_enc_buffer[SERIAL_BUF_SIZE];
  uint8_t MY_SSDV_img_chunk[IMG_CHUNK_SIZE_FOR_SSDV];
  int c;

  Serial.print("send_image_ssdv()\n");

  // initialise ssdv config structure
  // SSDV_TYPE_NORMAL or SSDV_TYPE_NOFEC, could turn off as LoRa includes some FEC 
  if (ssdv_enc_init(&ssdv, SSDV_TYPE_NORMAL, MY_SSDV_CALLSIGN, MY_SSDV_image_id, MY_SSDV_quality, MY_SSDV_packet_length) != SSDV_OK)
    return (0);
  ssdv_enc_set_buffer(&ssdv, MY_SSDV_enc_buffer);

  print_buf_len = sprintf(print_buf, "ALIF-BEGIN-SSDV-i%d-q%d-l%d\n", MY_SSDV_image_id, MY_SSDV_quality, MY_SSDV_packet_length);
#ifdef USE_MIN 
  min_send_frame(&min_ctx, INFO_CHANNEL, (uint8_t *)print_buf, (uint8_t)print_buf_len); 
#else
  ECHOT7.print(print_buf);
#endif

  while (1) {
    while ((c = ssdv_enc_get_packet(&ssdv)) == SSDV_FEED_ME) {
      // read packet worth of bytes from image buffer
      i_index += fill_ssdv_buffer(MY_SSDV_img_chunk, IMG_CHUNK_SIZE_FOR_SSDV, source_image_buf, source_image_size, i_index);
      Serial.printf("Feeding SSDV Encoder, i_index = %d\n", i_index);

      // ssdv =  struct, imageBuffer = buffer containing image packet, r = size
      ssdv_enc_feed(&ssdv, MY_SSDV_img_chunk, IMG_CHUNK_SIZE_FOR_SSDV);
    }
        
    if (c == SSDV_EOI) {
      Serial.print("ssdv_enc_get_packet said EOI\n");
      break;
    }
    else if (c != SSDV_OK) {
      Serial.printf("ssdv_enc_get_packet failed: %d\n", c);
      MY_SSDV_image_id++;
      return (0);
    } 

    // waiting between each SSDV packet
    while ((my_get_time_ms() - ssdv_last_sent < 5000U)) {
#ifdef USE_MIN      
      local_min_poll();
#endif      
      delay(500);
    }
#ifdef USE_MIN    
    // we call min_poll in case
    local_min_poll();
#endif    

    Serial.printf("--> Sending SSDV packet #%d to EchoT7...\n", MY_SSDV_packet_count);

#ifdef DISPLAY_SSDV_PACKETS
    for (uint8_t i=0; i< MY_SSDV_packet_length; i++) {
      //if (MY_SSDV_enc_buffer[i] < 0x10)
      //  Serial_print("0");
      Serial.printf("%.2X", MY_SSDV_enc_buffer[i]);
    }

    Serial.print("\n");
#endif

#ifdef USE_MIN    
    // Send a MIN frame with ID 0x01. The payload will be in this machine's
    // endianness - i.e. little endian - and so the host code will need to flip the bytes
    // around to decode it. It's a good idea to stick to MIN network ordering (i.e. big
    // endian) for payload words but this would make this example program more complex.
#ifdef TRANSPORT_PROTOCOL
    Serial.print("with T_MIN\n");
    if (!min_queue_has_space_for_frame(&min_ctx, (uint8_t)MY_SSDV_packet_length)) {
      Serial.print("No more space, resetting at time ");
      Serial.println(my_get_time_ms());
      min_transport_reset(&min_ctx, true);
    }
    // normally should not happen
    if (!min_queue_frame(&min_ctx, SSDV_DATA_CHANNEL, (uint8_t *)MY_SSDV_enc_buffer, (uint8_t)MY_SSDV_packet_length)) {
      // The queue has overflowed for some reason
      Serial.print("Can't queue at time ");
      Serial.println(my_get_time_ms());
    }
#else
    Serial.print("with MIN\n");
    min_send_frame(&min_ctx, SSDV_DATA_CHANNEL, (uint8_t *)MY_SSDV_enc_buffer, (uint8_t)MY_SSDV_packet_length);    
#endif
#else // USE_MIN
    // use simple print to serial port
    ECHOT7.print((char*)MY_SSDV_enc_buffer);    
#endif

    MY_SSDV_packet_count++;
    ssdv_last_sent = my_get_time_ms();

    // in case we get stuck in this loop
    if (MY_SSDV_packet_count > MY_SSDV_max_packet_num) {
      Serial.printf("ERROR: MY_SSDV_packet_count > %d\n", MY_SSDV_max_packet_num);
      break; 
    }    
  }

  Serial.printf("Sent %d SSDV packets\n", MY_SSDV_packet_count);

  print_buf_len = sprintf(print_buf, "ALIF-END-SSDV-i%d-q%d-l%d-p%d\n", MY_SSDV_image_id, MY_SSDV_quality, MY_SSDV_packet_length, MY_SSDV_packet_count);
#ifdef USE_MIN 
  min_send_frame(&min_ctx, INFO_CHANNEL, (uint8_t *)print_buf, (uint8_t)print_buf_len); 
#else
  ECHOT7.print(print_buf);
#endif
  MY_SSDV_image_id++;  
  return (1);
}

// This is used to keep track of when the next example message will be sent
uint32_t last_sent;
uint16_t count_sent=0;

void setup() {
  Serial.begin(115200);
  ECHOT7.begin(ECHOT7_BAUD, SERIAL_8N1, RX_PIN, TX_PIN);
  delay(3000);

  // BEWARE that if we look until Serial is available
  // it is blocking if not connected to a computer!
  //while(!Serial) {
  //  ; // Wait for serial port
  //}
  
  // Wait for USB serial port to connect. Note that this won't return until the host PC
  // opens the USB serial port.
  //while(!ECHOT7) {
  //  ;
  //}

#ifdef USE_MIN
  // Initialize the single context. Since we are going to ignore the port value we could
  // use any value. But in a bigger program we would probably use it as an index.
  min_init_context(&min_ctx, 0);
#endif

#ifdef USE_MIN
  Serial.print("Using Microcontroller Interconnect Network (MIN)\n");
#ifdef TRANSPORT_PROTOCOL
  Serial.print("with T_MIN transport protocol\n");
#endif  
#endif

  Serial.print("Starting...\n");
  last_sent = my_get_time_ms();
}

void loop() {
  char buf[SERIAL_BUF_SIZE];
  uint8_t c;
  uint32_t buf_len=0;

#ifdef USE_MIN  
  local_min_poll();
#endif

  uint32_t now = my_get_time_ms();

#ifdef USE_TEST_JPG_YCRCB_ARRAY_TO_SSDV
  // Every 60s send an SSDV image using the reliable transport stream
  // Use modulo arithmetic so that it will continue to work when the time value wraps
  if (now - last_sent > 15000U) {
    send_image_ssdv((uint8_t*)person240x240_YCrCb, sizeof(person240x240_YCrCb));
    last_sent = my_get_time_ms();  
  }
#else // USE_TEST_JPG_YCRCB_ARRAY_TO_SSDV
  // Every 5s send test message
  // Use modulo arithmetic so that it will continue to work when the time value wraps
  if (now - last_sent > 5000U) {
#ifdef USE_TEST_SSDV_PKT
    buf_len = sizeof(example_ssdv_packet);
    memcpy(buf, example_ssdv_packet, buf_len);
    Serial_print("Sending binary test SSDV packet to EchoT7...\n");
#else
    buf_len = sprintf(buf, "hello #%d from ESP32S3\n", count_sent++);
    Serial_print("Sending hello to EchoT7...\n");
    Serial_println(buf);
#endif

#ifdef USE_MIN    
    // Send a MIN frame with ID 0x01. The payload will be in this machine's
    // endianness - i.e. little endian - and so the host code will need to flip the bytes
    // around to decode it. It's a good idea to stick to MIN network ordering (i.e. big
    // endian) for payload words but this would make this example program more complex.
#ifdef TRANSPORT_PROTOCOL
    Serial.print("with T_MIN\n");
    if (!min_queue_has_space_for_frame(&min_ctx, (uint8_t)buf_len)) {
      Serial.print("No more space, resetting at time ");
      Serial.println(my_get_time_ms());
      min_transport_reset(&min_ctx, true);
    }
    // normally should not happen
    if (!min_queue_frame(&min_ctx, SSDV_DATA_CHANNEL, (uint8_t *)buf, (uint8_t)buf_len)) {
      // The queue has overflowed for some reason
      Serial.print("Can't queue at time ");
      Serial.println(my_get_time_ms());
    }
#else
    Serial.print("with MIN\n");
    min_send_frame(&min_ctx, SSDV_DATA_CHANNEL, (uint8_t *)buf, (uint8_t)buf_len);    
#endif
#else // USE_MIN
    // use simple print to serial port
    ECHOT7.print((char*)buf);    
#endif
    last_sent = now;
  }
#endif // USE_TEST_JPG_YCRCB_ARRAY_TO_SSDV 
}
