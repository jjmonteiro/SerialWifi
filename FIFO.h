#ifndef MEMORY_BUFFER_H
#define MEMORY_BUFFER_H

	/*****************************************************************************/
	/*	Serial Debugger over Wifi - UP840126 - University of Portsmouth
	/*****************************************************************************/
	static const int  BUFFER_SIZE = 5512;			//Bytes (x2 + 4K < 20KB)


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
	/*	This member function returns the contents of the whole buffer
	/*****************************************************************************/
	String ReadStringFromBuffer() 
	{
		String SavedData = "";

		if (IsBufferFull == true){
			SavedData = ReadStringFromRange(CurrentBufferPosition, BUFFER_SIZE);
		}

		if (CurrentBufferPosition != 0) {
			SavedData += ReadStringFromRange(0, CurrentBufferPosition);
		}

		return SavedData;
	}

	/*****************************************************************************/
	/*	This member function returns a range of contents from the buffer
	/*****************************************************************************/
	String  ReadStringFromRange(unsigned int StartPosition, unsigned int EndPostion)
	{
		String Result = "";
		if (StartPosition < EndPostion) //EndPosition will never be returned!
		{
			for (unsigned int Index = StartPosition; Index < EndPostion; Index++){
				Result += char(MEMORY_BUFFER[Index]);
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