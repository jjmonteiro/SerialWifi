#ifndef MEMORY_BUFFER_H
#define MEMORY_BUFFER_H

	/*****************************************************************************/
	/*	Serial Debugger over Wifi - UP840126 - University of Portsmouth
	/*****************************************************************************/
	static const int  BUFFER_SIZE = 6000;			//Bytes (x2 + 4K < 20KB)
	const char HEX_ARRAY[] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F' };

	struct MemoryBuffer 
	{

		unsigned int CurrentBufferPosition;
		byte MEMORY_BUFFER[BUFFER_SIZE];
		bool IsBufferFull;

	/*****************************************************************************/
	/*	This member function initializes the buffer
	/*****************************************************************************/
	MemoryBuffer()
	{
		IsBufferFull = false;
		CurrentBufferPosition = 0;

		for (int Index = 0; (Index < BUFFER_SIZE); Index++){
			MEMORY_BUFFER[Index] = 0;
		}
	}

	/*****************************************************************************/
	/*	This member function returns the current position of the buffer
	/*****************************************************************************/
	unsigned int GetCurrentPosition()
	{
		return CurrentBufferPosition;
	}

	/*****************************************************************************/
	/*	This member function returns the current size of the buffer
	/*****************************************************************************/
	unsigned int GetCurrentSize()
	{
		if (IsBufferFull == true) {
			return BUFFER_SIZE;
		}
		else {
			return CurrentBufferPosition;
		}
	}

	/*****************************************************************************/
	/*	This member function converts one byte to a displayable text string
	/*****************************************************************************/
	String ConvertByteToString(byte data, int datatype)
	{
		unsigned int nibble = data;
		String output = String(HEX_ARRAY[nibble >> 4] + HEX_ARRAY[0x0F & nibble]);

		if (datatype) {	//as hex text ex:" 3C"
			return (" " + output);
		}
		else {	//as html text ex "&#x3C;"
			return ("&#x" + output);
		}
	}

	/*****************************************************************************/
	/*	This member function returns the contents of the whole buffer
	/*****************************************************************************/
	String ReadStringFromBuffer(int datatype) 
	{
		String SavedData = "";

		if (IsBufferFull == true){
			SavedData = ReadStringFromRange(CurrentBufferPosition, BUFFER_SIZE, datatype);
		}

		if (CurrentBufferPosition != 0) {
			SavedData += ReadStringFromRange(0, CurrentBufferPosition, datatype);
		}

		return SavedData;
	}

	/*****************************************************************************/
	/*	This member function returns a range of contents from the buffer
	/*****************************************************************************/
	String  ReadStringFromRange(unsigned int StartPosition, unsigned int EndPostion, int datatype)
	{
		String Result = "";
		if (StartPosition < EndPostion) //EndPosition will never be returned!
		{
			for (unsigned int Index = StartPosition; Index < EndPostion; Index++){
				Result += ConvertByteToString(MEMORY_BUFFER[Index], datatype);
			}
		}
		return  Result;
	}

	/*****************************************************************************/
	/*	This member function writes a String into the buffer
	/*****************************************************************************/
	void WriteStringToBuffer(String NewData)
	{

		for (unsigned int Index = 0; Index < NewData.length(); Index++) {
			MEMORY_BUFFER[CurrentBufferPosition++] = NewData.charAt(Index);
			
			if (CurrentBufferPosition >= BUFFER_SIZE)
			{
				CurrentBufferPosition = 0;
				IsBufferFull = true;
			}
		}
	}

	/*****************************************************************************/
	/*	This member function writes a byte into the buffer
	/*****************************************************************************/
	void WriteByteToBuffer(byte NewData)
	{

		//Serial.println("Line " + String(CurrentBufferPosition) + " :: ");

		MEMORY_BUFFER[CurrentBufferPosition++] = NewData;

		if (CurrentBufferPosition >= BUFFER_SIZE)
		{
			CurrentBufferPosition = 0;
			IsBufferFull = true;
		}
	}

};

#endif