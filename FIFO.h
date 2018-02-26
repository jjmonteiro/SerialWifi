#ifndef MEMORY_BUFFER_H
#define MEMORY_BUFFER_H

extern String freeHeap;
extern String powerSupply;

	/*****************************************************************************/
	/*	Serial Debugger over Wifi - UP840126 - University of Portsmouth
	/*****************************************************************************/

	struct MemoryBuffer {

		unsigned int CurrentBufferPosition;
		char MEMORY_BUFFER[MAX_LINES][MAX_CHARS];
		bool IsBufferFull;

	/*****************************************************************************/
	/*	This member function initializes the buffer
	/*****************************************************************************/
	MemoryBuffer()
	{
		IsBufferFull = false;
		CurrentBufferPosition = 0;

		for (int a = 0; (a < MAX_LINES); a++)
		{
			for (int b = 0; (b < MAX_CHARS); b++)
			{
				MEMORY_BUFFER[a][b] = 0;
			}
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
	/*	This member function returns the contents of the whole buffer
	/*****************************************************************************/
	String ReadStringFromBuffer() {
		String SavedData = "";
		Serial.println("Buffer ReadAll Call");

		Serial.print("FullLOG: "); Serial.println(IsBufferFull);
		Serial.print("nextLine: "); Serial.println(CurrentBufferPosition);

		if (IsBufferFull == true){
			SavedData = ReadStringFromRange(CurrentBufferPosition, MAX_LINES);
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
		if (StartPosition < EndPostion)
		{
			for (unsigned int Index = StartPosition; Index < EndPostion; Index++)
			{
				Result += MEMORY_BUFFER[Index];
				Serial.print(Index, DEC);
			}
		}
		return  Result;
	}

	/*****************************************************************************/
	/*	This member function writes a String into the buffer
	/*****************************************************************************/
	void WriteStringToBuffer(String NewData)
	{
		Serial.print("saving..");
		Serial.println(freeHeap + "::" + powerSupply);
		String Data = (String(millis()) + ": " + NewData);
		Data.toCharArray(MEMORY_BUFFER[CurrentBufferPosition], MAX_CHARS);///code breaks here
		CurrentBufferPosition++;
		Serial.println("saved!");
		
		if (CurrentBufferPosition >= MAX_LINES)
		{
			CurrentBufferPosition = 0;
			IsBufferFull = true;
			Serial.println("rolling over");
		}
	}

};


#endif