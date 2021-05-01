/*
*
* Este código permite registrar la entrada o salida de un tarjeta de RFID
*
*
*/

#include <SPI.h>
#include <MFRC522.h>
#include <stdint.h>

#define RST_PIN         22           // Configurable, see typical pin layout above
#define SS_PIN          21          // Configurable, see typical pin layout above
#define RETRASO       250

void entrada_producto();
void salida_producto();
void grabar_caducidad();
void mostrar_menu();
void limpiar_terminal();
void mfrc_write();
void mfrc_read();
byte serial_echo( byte * data);

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance
//*****************************************************************************************//
void setup() {
  Serial.begin(9600);                                           // Initialize serial communications with the PC
  SPI.begin();                                                  // Init SPI bus
  mfrc522.PCD_Init();                                              // Init MFRC522 card
  Serial.println(F(" Smart inventory Frontend "));    //shows in serial that it is ready to read
}

//*****************************************************************************************//
//Esta función muestra un menú de acción que se actualiza cada que se termina la acción solicitada
void loop()
{
  mostrar_menu();

/**
  byte  data[30]= {};
  byte len;
  len = serial_echo(data);

  Serial.println(len);
*/
}

void mostrar_menu()
{
    byte tarea_elegida;
    byte buffer[1];

    limpiar_terminal();

    Serial.println("Selecciona la tarea que deseas ejecutar: ");

    Serial.println(" ");
    Serial.println(" 1. Registrar entrada  ");
    Serial.println(" 2. Registrar salida ");
    Serial.println(" 3. Grabar info de producto ");
    Serial.println(" - - - - - - - - - - - - - - - - - - - - - - -");

    
    while(!Serial.available()) ; // hang program until a byte is received notice the ; after the while()

    tarea_elegida = Serial.peek();
    Serial.println( tarea_elegida - 48 );
    Serial.flush(); //flush all previous received and transmitted data

    switch (tarea_elegida)
    {
      case '1':
      
        entrada_producto();
        break;
    case '2':
        salida_producto();
        break;
    case '3':
        grabar_caducidad();
        break;
    
    default:
        Serial.println("Valor invalido. Regresando al menú princial...");
        break;
    }
}
void entrada_producto()
{
    limpiar_terminal();
    Serial.println("Registrando ENTRADA del producto...");
    mfrc_read();
    
}

void salida_producto()
{
    Serial.println("Registrando SALIDA del producto");
    mfrc_read();
   
}

void grabar_caducidad()
{
    //Serial.println("Introduzca fecha de caducidad");

    mfrc_write();
    
   
}

void limpiar_terminal()
{
  delay(RETRASO);
   Serial.write(27);       // ESC command
  Serial.print("[2J");    // clear screen command
  Serial.write(27);
  Serial.print("[H");     // cursor to home command
}

void mfrc_write()
{
// Prepare key - all keys are set to FFFFFFFFFFFFh at chip delivery from the factory.
  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Select one of the cards/*
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  Serial.print(F("Card UID:"));    //Dump UID
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
  }
  Serial.print(F(" PICC type: "));   // Dump PICC type
  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  Serial.println(mfrc522.PICC_GetTypeName(piccType));

 
  limpiar_terminal();

  byte buffer[34];
  byte block;
  MFRC522::StatusCode status;
  byte len;

  Serial.setTimeout(20000L) ;     // wait until 20 seconds for input from serial
  // Introduce la caducidad del producto
  Serial.println(F(" Ingresa NOMBRE o SKU del producto. "));

  len = serial_echo( buffer);
  //len = Serial.readBytesUntil('.', (char *) buffer, 30) ; // read family name from serial


  for (byte i = len; i < 30; i++) buffer[i] = ' ';     // pad with spaces

  block = 1;
  //Serial.println(F("Authenticating using key A..."));
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  else Serial.println(F("PCD_Authenticate() success: "));

  // Write block
  status = mfrc522.MIFARE_Write(block, buffer, 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Write() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  else Serial.println(F("MIFARE_Write() success: "));

  block = 2;
  //Serial.println(F("Authenticating using key A..."));
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  // Write block
  status = mfrc522.MIFARE_Write(block, &buffer[16], 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Write() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  else Serial.println(F("MIFARE_Write() success: "));

 
  limpiar_terminal();

  // Pregunta el nomnbre del producto
  Serial.println(F(" Ingresa la fecha de caducidad "));
    //len = Serial.readBytesUntil('.', (char *) buffer, 20) ; // read first name from serial

  len =  serial_echo( buffer);
  for (byte i = len; i < 20; i++) buffer[i] = ' ';     // pad with spaces

  block = 4;
  //Serial.println(F("Authenticating using key A..."));
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  // Write block
  status = mfrc522.MIFARE_Write(block, buffer, 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Write() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  else Serial.println(F("MIFARE_Write() success: "));

  block = 5;
  //Serial.println(F("Authenticating using key A..."));
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  // Write block
  status = mfrc522.MIFARE_Write(block, &buffer[16], 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Write() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  else Serial.println(F("MIFARE_Write() success: "));


  Serial.println(" ");
  mfrc522.PICC_HaltA(); // Halt PICC
  mfrc522.PCD_StopCrypto1();  // Stop encryption on PCD
}

void mfrc_read()
{
   // Prepare key - all keys are set to FFFFFFFFFFFFh at chip delivery from the factory.
  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  //some variables we need
  byte block;
  byte len;
  MFRC522::StatusCode status;

  //-------------------------------------------

  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }

 // Serial.println(F("**Card Detected:**"));

  //-------------------------------------------

  mfrc522.PICC_DumpDetailsToSerial(&(mfrc522.uid)); //dump some details about the card

  //mfrc522.PICC_DumpToSerial(&(mfrc522.uid));      //uncomment this to see all blocks in hex

  //-------------------------------------------
  limpiar_terminal();

  Serial.println("");
  Serial.println("");
  Serial.println("****************");
  Serial.print("CADUCIDAD:    ");
  

  byte buffer1[18];

  block = 4;
  len = 18;

  //------------------------------------------- Nombre del producto
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 4, &key, &(mfrc522.uid)); //line 834 of MFRC522.cpp file
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Authentication failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  status = mfrc522.MIFARE_Read(block, buffer1, &len);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Reading failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  //Imprime el nombre del producto
  for (uint8_t i = 0; i < 16; i++)
  {
    if (buffer1[i] != 32)
    {
      Serial.write(buffer1[i]);
    }
  }
  Serial.println(" ");
  Serial.print("PRODUCTO:  ");
  

  //---------------------------------------- GET LAST NAME

  byte buffer2[18];
  block = 1;

  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 1, &key, &(mfrc522.uid)); //line 834
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Authentication failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  status = mfrc522.MIFARE_Read(block, buffer2, &len);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Reading failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  //Imprime fecha de caducidad del producto
  for (uint8_t i = 0; i < 16; i++) {
    Serial.write(buffer2[i] );
  }


  //----------------------------------------

  //Serial.println(F("\n**End Reading**\n"));

  delay(1000); //change value if you want to read cards faster

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}

//Echoes the entered data through serial terminal
byte serial_echo( byte * data)
{
  while(!Serial.available() );


  //byte data[30] = {}; //serial character buffer
  //byte len; //number of characters stored in the buffer

  byte  index = 0;

  while( 1  )
  {
    
     while(!Serial.available() );
    data[index] = Serial.read();
    Serial.print((char) data[index]);
     if('.' == (char)data[index])
    {
      break;
    }
    

    index = index+ 1;
    index = index %30;

   
  }
  Serial.println(" ");
  Serial.println("DATOS REGISTRADOS");
  return index;

}