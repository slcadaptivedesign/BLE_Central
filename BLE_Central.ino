/*********************************************************************
  This is an example for our nRF52 based Bluefruit LE modules
  Pick one up today in the adafruit shop!
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!
  MIT license, check LICENSE for more information
  All text above, and the splash screen below must be included in
  any redistribution
*********************************************************************/

/*
   This sketch demonstrate the central API(). A additional bluefruit
   that has bleuart as peripheral is required for the demo.
*/
#include <bluefruit.h>

BLEClientDis  clientDis;
BLEClientUart clientUart;

const int buttonPress = 11;
int change = 0;
int state = 0;

void setup()
{
  Serial.begin(115200);
  pinMode(27, OUTPUT);
  pinMode(11, INPUT);
  Serial.println("Bluefruit52 Central BLEUART Example");
  Serial.println("-----------------------------------");

  // Enable both peripheral and central
  Bluefruit.begin(true, true);
  Bluefruit.setName("Bluefruit1000");

  // Configure DIS client
  clientDis.begin();

  // Init BLE Central Uart Serivce
  clientUart.begin();
  clientUart.setRxCallback(uart_rx_callback);

  // Increase Blink rate to different from PrPh advertising mode
  Bluefruit.setConnLedInterval(250);

  // Callbacks for Central
  Bluefruit.Central.setConnectCallback(connect_callback);
  Bluefruit.Central.setDisconnectCallback(disconnect_callback);

  // Start Central Scan
  Bluefruit.Central.setScanCallback(scan_callback);
  Bluefruit.Central.startScanning();
}

/**
   Callback invoked when scanner pick up an advertising data
   @param report Structural advertising data
*/
void scan_callback(ble_gap_evt_adv_report_t* report)
{
  // Check if advertising contain BleUart service
  if ( Bluefruit.Central.checkUuidInScan(report, BLEUART_UUID_SERVICE) )
  {
    Serial.println("BLE UART service detected");
    Serial.println("Attempt to connect ... ");

    // Connect to device with bleuart service in advertising
    // Use Min & Max Connection Interval default value
    Bluefruit.Central.connect(report);
  }
}

/**
   Callback invoked when an connection is established
   @param conn_handle
*/
void connect_callback(uint16_t conn_handle)
{
  Serial.println("Connected");

  Serial.print("Dicovering DIS ... ");
  if ( clientDis.discover(conn_handle) )
  {
    Serial.println("Found it");
    char buffer[32 + 1];

    // read and print out Manufacturer
    memset(buffer, 0, sizeof(buffer));
    if ( clientDis.getManufacturer(buffer, sizeof(buffer)) )
    {
      Serial.print("Manufacturer: ");
      Serial.println(buffer);
    }

    // read and print out Model Number
    memset(buffer, 0, sizeof(buffer));
    if ( clientDis.getModel(buffer, sizeof(buffer)) )
    {
      Serial.print("Model: ");
      Serial.println(buffer);
    }

    Serial.println();
  }

  Serial.print("Discovering BLE Uart Service ... ");

  if ( clientUart.discover(conn_handle) )
  {
    Serial.println("Found it");

    Serial.println("Enable TXD's notify");
    clientUart.enableTXD();

    Serial.println("Ready to receive from peripheral");
  } else
  {
    Serial.println("Found NONE");
  }
}

/**
   Callback invoked when a connection is dropped
   @param conn_handle
   @param reason
*/
void disconnect_callback(uint16_t conn_handle, uint8_t reason)
{
  (void) conn_handle;
  (void) reason;

  Serial.println("Disconnected");
  Serial.println("Bluefruit will auto start scanning (default)");
}

void uart_rx_callback(void)
{
  Serial.print("[RX]: ");

  while ( clientUart.available() )
  {
    //char rx = (char) clientUart.read();
    Serial.print((char) clientUart.read() );
  }

  Serial.println();
}

void loop()
{
  if ( Bluefruit.Central.connected() )
  {
    // Not discovered yet
    if ( clientUart.discovered() )
    {
      if (digitalRead(buttonPress) == HIGH)
      {
        state = 1;
      }

      else {
        state = 0;
      }

      if (state != change) {
        if (state == 1) {
          digitalWrite(27, HIGH);
          clientUart.print("F");
        }
        else {
          digitalWrite(27, LOW);
          clientUart.print("S");
          // Delay a bit after a report
          delay(5);
        }

        change = state;
        /*
          if (digitalRead(11) == HIGH){
          clientUart.print("H");
          delay(10000);
          }
        */
        // Discovered means in working state
        // Get Serial input and send to Peripheral
        if ( Serial.available() )
        {
          delay(2); // delay a bit for all characters to arrive

          char str[20 + 1] = { 0 };
          Serial.readBytes(str, 20);

          clientUart.print( str );
        }
      }
    }
  }
}
