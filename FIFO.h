/*****************************************************************************/
/*	Serial Debugger over Wifi - UP840126 - University of Portsmouth
/*****************************************************************************/

typedef struct {
	char BUFFER[MAX_LINES][MAX_CHARS];
	int nextLine;
	bool fullLOG;

/*****************************************************************************/
/*	This member function clears all variables and all data in the buffer
/*****************************************************************************/
	void Init() {
		Serial.println("Buffer Init Call");
		fullLOG = false;
		nextLine = 0;

		for (int a=0; (a< MAX_LINES); a++)
			for (int b = 0; (b < MAX_CHARS); b++)
				BUFFER[a][b] = 0;

	}
	/*void swipe() {
		Serial.println("Buffer swipe Call");

		for (int a = 0; (a < MAX_LINES); a++)
			for (int b = 0; (b < MAX_CHARS); b++)
				Serial.print(char(BUFFER[a][b]));

	}*/
/*****************************************************************************/
/*	This member function returns the contents of the buffer
/*****************************************************************************/
	void ReadAll (String &SavedData) {
		Serial.println("Buffer ReadAll Call");
		
		SavedData = "";

		if (fullLOG == true) {
			for (int n = nextLine; n < MAX_LINES; n++)
				SavedData += BUFFER[n];
		}

		if (nextLine != 0) {
			for (int m = 0; m < nextLine; m++)
				SavedData += BUFFER[m];
		}
	}

/*****************************************************************************/
/*	This member function a String into the buffer - don't forget termination!
/*****************************************************************************/
	void WriteLine (String NewData) {
		Serial.println("Buffer PrintLine Call");

		(String(millis()) + ": " + NewData).toCharArray(BUFFER[nextLine],MAX_CHARS);

		nextLine++;

		if (nextLine >= MAX_LINES) {
			nextLine = 0;
			fullLOG = true;
			Serial.println("rolling over");
		}

	}


} FIFO;
