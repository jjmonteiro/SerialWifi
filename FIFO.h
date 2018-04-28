  /*****************************************************************************/
  /*  Serial Debugger over Wifi - UP840126 - University of Portsmouth
  /*****************************************************************************/

#ifndef FIFO_H
#define FIFO_H

static const size_t  BUFFER_SIZE = 4000;      //Bytes (x2 + 4K < 20KB)

struct MemoryBuffer
{

  size_t CurrentBufferPosition;
  byte MEMORY_BUFFER[BUFFER_SIZE];
  bool IsBufferFull;

  /*****************************************************************************/
  /*  This member function initializes the buffer
  /*****************************************************************************/
  MemoryBuffer()
  {
    IsBufferFull = false;
    CurrentBufferPosition = 0;

    for (size_t Index = 0; (Index < BUFFER_SIZE); Index++) {
      MEMORY_BUFFER[Index] = 0;
    }
  }

  /*****************************************************************************/
  /*  This member function returns the current position of the buffer
  /*****************************************************************************/
  size_t GetCurrentPosition()
  {
    return CurrentBufferPosition;
  }

  /*****************************************************************************/
  /*  This member function resets the current position of the buffer
  /*****************************************************************************/
  size_t ResetCurrentPosition()
  {
    IsBufferFull = false;
    CurrentBufferPosition = 0;
  }
  /*****************************************************************************/
  /*  This member function returns the current size of the buffer
  /*****************************************************************************/
  size_t GetCurrentSize()
  {
    if (IsBufferFull) {
      return BUFFER_SIZE;
    }
    else {
      return CurrentBufferPosition;
    }
  }

  /*****************************************************************************/
  /*  This member function returns a range of contents from the buffer
  /*****************************************************************************/
  String  ReadStringFromRange(size_t StartPosition, size_t EndPostion)
  {
    String Result, temp = "";

    if (StartPosition < EndPostion)           //EndPosition will never be returned!
    {
      for (size_t Index = StartPosition; Index < EndPostion; Index++) {
        Result += GetByteAsString(Index);       //get byte at index as hex string
      }
    }
    return  Result;
  }

  /*****************************************************************************/
  /*  This member function returns a byte from the buffer as String
  /*****************************************************************************/
  String  GetByteAsString(size_t Position)
  {
    String HexChar = "";
    if (Position >= 0 && Position < BUFFER_SIZE) {
      HexChar = String(MEMORY_BUFFER[Position], HEX); //get byte at index and convert to hex string
      MEMORY_BUFFER[Position] = 0;         //clear Position
      if (HexChar.length() < 2) {           //because '0a' will be returned as 'a'
        HexChar = "0" + HexChar;
      }
    }
    return  HexChar;
  }

  /*****************************************************************************/
  /*  This member function writes a String into the buffer
  /*****************************************************************************/
  void WriteStringToBuffer(String NewData)
  {

    for (size_t Index = 0; Index < NewData.length(); Index++) {
      WriteByteToBuffer(NewData.charAt(Index));
    }
  }

  /*****************************************************************************/
  /*  This member function writes a byte into the buffer
  /*****************************************************************************/
  void WriteByteToBuffer(byte NewData)
  {

    MEMORY_BUFFER[CurrentBufferPosition++] = NewData;

    if (CurrentBufferPosition >= BUFFER_SIZE)
    {
      CurrentBufferPosition = 0;
      IsBufferFull = true;
    }
  }
};

#endif
