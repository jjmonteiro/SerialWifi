/*****************************************************************************/
/*	Serial Debugger over Wifi - UP840126 - University of Portsmouth
/*****************************************************************************/
#ifndef MEMORY_BUFFER_H
#define MEMORY_BUFFER_H

struct MemoryBuffer
{

	MemoryBuffer()
	{
		Serial.println("Buffer Init Call");
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
	
	unsigned int GetCurrentPosition()
	{
		return CurrentBufferPosition;
	}
	/*****************************************************************************/
	/*	This member function returns the contents of the buffer
	/*****************************************************************************/
	void ReadStringFromBuffer(String &SavedData) {
		Serial.println("Buffer ReadAll Call");

		//SavedData = "";
		Serial.println("SavedData cleared");
		Serial.print("FullLOG: "); Serial.println(IsBufferFull);
		Serial.print("nextLine: "); Serial.println(CurrentBufferPosition);

		if (IsBufferFull == true) 
		{
			SavedData = ReadStringFromRange(CurrentBufferPosition, MAX_LINES);
		}
		Serial.println("SavedData 2nd Stage");
		SavedData += ReadStringFromRange(0, CurrentBufferPosition);
		Serial.println("SavedData complete!");
	}



String  ReadStringFromRange(unsigned int StartPosition, unsigned int EndPostion)
	{
	  String Result = "";
		if ((StartPosition < MAX_LINES) && (StartPosition < EndPostion))
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
/*	This member function a String into the buffer - don't forget termination!
/*****************************************************************************/
	void WriteStringToBuffer(String NewData)
	{
		if (CurrentBufferPosition < MAX_LINES)
		{
			CurrentBufferPosition = 0;
			IsBufferFull = true;
			Serial.println("rolling over");
		}
		Serial.println("Buffer PrintLine Call");
		String Data = (millis() + ": " + NewData);
		Data.toCharArray(MEMORY_BUFFER[CurrentBufferPosition], MAX_CHARS);
		CurrentBufferPosition++;
	}

	

private:
	char         MEMORY_BUFFER[MAX_LINES][MAX_CHARS];
	unsigned int CurrentBufferPosition;
	bool         IsBufferFull;
};


#endif