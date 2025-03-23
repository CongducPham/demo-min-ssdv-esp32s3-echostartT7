#ifdef ARDUINO_XIAO_ESP32S3
#define USB_SERIAL      Serial
// on the EchoStar Terminal T7, HEADER_SERIAL is also defined as Seria1
// see leat-echostar-terminals/hardware/stm32/12.7.5-0/variants/variant_EchoT7/variant_generic.h
#define HEADER_SERIAL   Serial1
#define RX_PIN          D7
#define TX_PIN          D6
#define HEADER_BAUD     115200
#endif

//#define ENABLE_SENDER
#define USE_MIN
//#define ENABLE_MIN_DEBUG
//#define MIN_SHOW_DEBUG_1
//#define MIN_SHOW_DEBUG_2
#define MIN_SHOW_HEXSTRING

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
  USB_SERIAL.print(debug_print_buf);
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
  uint16_t n = HEADER_SERIAL.availableForWrite();

  return n;
}

// Send a character on the designated port.
// Here we would send using HEADER_SERIAL which communicates with the ESP32S3
void min_tx_byte(uint8_t port, uint8_t byte)
{
  // Ignore 'port' because we have just one context.
  HEADER_SERIAL.write(&byte, 1U);  
}

// Tell MIN the current time in milliseconds.
uint32_t min_time_ms(void)
{
  return millis();
}

void min_tx_start(uint8_t port) {
#ifdef MIN_SHOW_DEBUG_2
#ifdef TRANSPORT_PROTOCOL
  USB_SERIAL.print("T_MIN TX start\n");
#else
  USB_SERIAL.print("MIN TX start\n");
#endif  
#endif
}

void min_tx_finished(uint8_t port) {
#ifdef MIN_SHOW_DEBUG_2  
#ifdef TRANSPORT_PROTOCOL
  USB_SERIAL.print("T_MIN TX finished\n");
#else
  USB_SERIAL.print("MIN TX finishedn");
#endif 
#endif
}

// Handle the reception of a MIN frame. This is the main interface to MIN for receiving
// frames. It's called whenever a valid frame has been received (for transport layer frames
// duplicates will have been eliminated).
void min_application_handler(uint8_t min_id, uint8_t const *min_payload, uint8_t len_payload, uint8_t port)
{
  // We ignore the port because we have one context, but we could use it to index an array of
  // contexts in a bigger application.
  USB_SERIAL.print("MIN frame with ID ");
  USB_SERIAL.print(min_id);
  USB_SERIAL.print(" received at ");
  USB_SERIAL.println(min_time_ms());

  if (min_payload[0]==0x55 && min_payload[1]==0x66)
    USB_SERIAL.println("SSDV packet, skip ASCII print");
  else  {
    for (uint8_t i=0; i< len_payload; i++) {
      USB_SERIAL.write((char)min_payload[i]);
    }
    USB_SERIAL.write((char)'\n');
  }

#ifdef MIN_SHOW_HEXSTRING
  // only for SSDV data
  if (min_payload[0]==0x55 && min_payload[1]==0x66) {
    USB_SERIAL.write((char)'>');
    for (uint8_t i=0; i< len_payload; i++) {
      if (min_payload[i] < 0x10)
        USB_SERIAL.write((char)'0');
      USB_SERIAL.print(min_payload[i], HEX);
    }
    USB_SERIAL.write((char)'\n');
  }
#endif
}
//////////////////////////////// END CALLBACKS /////////////////////////////////
#endif

// This is used to keep track of when the next example message will be sent
uint32_t last_sent;
uint16_t count_sent=0;

void setup(void)
{
  USB_SERIAL.begin(115200);
#ifdef ARDUINO_XIAO_ESP32S3    
  HEADER_SERIAL.begin(HEADER_BAUD, SERIAL_8N1, RX_PIN, TX_PIN);
#else
  HEADER_SERIAL.begin(HEADER_BAUD);
#endif

  delay(3000);

  // BEWARE that if we look until Serial is available
  // it is blocking if not connected to a computer!
  //while (!USB_SERIAL)
  //  ;

#ifdef USE_MIN 
  // Initialize the single context. Since we are going to ignore the port value we could
  // use any value. But in a bigger program we would probably use it as an index.
  min_init_context(&min_ctx, 0);
#endif

#ifdef USE_MIN
  USB_SERIAL.println("Using Microcontroller Interconnect Network (MIN)");
#ifdef TRANSPORT_PROTOCOL
  USB_SERIAL.println("with T_MIN transport protocol");
#endif  
#endif

  USB_SERIAL.println("Starting...");

  delay(1000);

  last_sent = millis();
}

void loop(void)
{
  char buf[32], recv_buf[255];
  uint8_t c;
  uint32_t buf_len=0, recv_buf_len=0;

  if (HEADER_SERIAL.available())
  {
    c = HEADER_SERIAL.read();
#ifdef USE_MIN    
    min_poll(&min_ctx, &c, (uint8_t)1);
#else
    USB_SERIAL.write((char)c);

    if (c == 0x0A || c == 0x0D)
    {
      USB_SERIAL.println("");
    }
#endif
  }
  else
    min_poll(&min_ctx, &c, 0);

#ifdef ENABLE_SENDER
  // Every 15s send a MIN frame using the reliable transport stream.
  uint32_t now = millis();

  // Use modulo arithmetic so that it will continue to work when the time value wraps
  if (now - last_sent > 15000U) {
    buf_len = sprintf(buf, "hello #%d from EchoT7\n", count_sent++);
    USB_SERIAL.println("Sending to ESP32S3...");
    USB_SERIAL.println(buf);
#ifdef USE_MIN    
    // Send a MIN frame with ID 0x01. The payload will be in this machine's
    // endianness - i.e. little endian - and so the host code will need to flip the bytes
    // around to decode it. It's a good idea to stick to MIN network ordering (i.e. big
    // endian) for payload words but this would make this example program more complex.

#ifdef TRANSPORT_PROTOCOL
    USB_SERIAL.println("with T_MIN");
    if (!min_queue_has_space_for_frame(&min_ctx, (uint8_t)buf_len)) {
      USB_SERIAL.print("No more space, resetting at time ");
      USB_SERIAL.println(millis());
      min_transport_reset(&min_ctx, true);
    }
    // normally should not happen
    if (!min_queue_frame(&min_ctx, 0x01, (uint8_t *)buf, (uint8_t)buf_len)) {
      // The queue has overflowed for some reason
      USB_SERIAL.print("Can't queue at time ");
      USB_SERIAL.println(millis());
    }
#else
    USB_SERIAL.println("with MIN");
    min_send_frame(&min_ctx, 0x01, (uint8_t *)buf, (uint8_t)buf_len);    
#endif
#else // USE_MIN
    HEADER_SERIAL.print(buf);    
#endif
    last_sent = now;
  }
#endif // ENABLE_SENDER
}
