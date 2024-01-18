//Pins for rotary switches
int load = A0;
int clockPulse = A1;
int dataOut = A2;
int r0 = A3;
int r1 = A4;
int r2 = A5;
int previous_position[] = {-1, -1, -1}; //Previous position of rotary switches

//Pins for buttons
int N_R[] = {
	3,
	8,
	9,
	2
};
int N_C[] = {
	7,
	10,
	4,
	5,
	6
};


int pb_matrix[4][5] = {
		{1, 1, 1, 1, 1},
		{1, 1, 1, 1, 1},
		{1, 1, 1, 1, 1},
		{1, 1, 1, 1, 1}};

//Messages to send via serial port when buttons are pressed
const String messages[4][5] = {
		{"", "", "", "", ""},
		{"PANForward=", "PANJog_up=", "", "", ""},
		{"PANJog_left=", "PANJog_right=", "", "", ""},
		{"PANBackward=", "PANJog_down=", "", "", ""}
		};

const char controllerIdentifier[] = "CNC Arduino controller";
const char startCommand[] = "START";

uint16_t value;

void setup()
{
		pinMode(load, OUTPUT);
		pinMode(clockPulse, OUTPUT);
		pinMode(dataOut, INPUT);
		pinMode(r2, INPUT);
		pinMode(r1, INPUT);
		pinMode(r0, INPUT);

		pinMode(N_R[0], INPUT_PULLUP);
		pinMode(N_R[1], INPUT_PULLUP);
		pinMode(N_R[2], INPUT_PULLUP);
		pinMode(N_R[3], INPUT_PULLUP);
		pinMode(N_C[0], INPUT_PULLUP);
		pinMode(N_C[1], INPUT_PULLUP);
		pinMode(N_C[2], INPUT_PULLUP);
		pinMode(N_C[3], INPUT_PULLUP);
		pinMode(N_C[4], INPUT_PULLUP);

		Serial.begin(115200);
		Serial.println("-------------------------------------------------------------------");
		Serial.println("Init done");
}


void read_selector(int sw)
{
		uint16_t dataIn = 0;
		int position = -1;
		digitalWrite(clockPulse, 0);
		digitalWrite(load, 0);
		delay(1);
		digitalWrite(clockPulse, 0);

		delay(1);
		digitalWrite(clockPulse, 1);
		delay(1);
		digitalWrite(load, 1);

		delay(1);

		for (int j = 15; j >= 0; j--)
		{
				digitalWrite(clockPulse, 1);
				delay(1);
				value = digitalRead(dataOut);
				if (value)
				{
						int a = (1 << j);

						dataIn = dataIn | a;
				}
				else
				{
						position = j;
				}

				digitalWrite(clockPulse, 0);
				delay(1);
		}
		if (previous_position[sw] == -1)
				previous_position[sw] = position;

		if (previous_position[sw] != position)
		{
				if (sw == 1)
				{
						if (previous_position[sw] > position)
						{
								Serial.println("PANJog_rate_next=1");
						}
						else
						{
								Serial.println("PANJog_rate_previous=1");
						}
				}
				else if (sw == 2)
				{
						if (previous_position[sw] > position)
						{
								Serial.println("PANJog_feed_next=1");
						}
						else
						{
								Serial.println("PANJog_feed_previous=1");
						}
				}
				previous_position[sw] = position;
		}
		delay(1);

		digitalWrite(clockPulse, 1);

		delay(25);
}





void read_pb_row(int row){
	pinMode(N_R[row], OUTPUT);
	digitalWrite(N_R[row], LOW);
	delay(2);

	for(int col = 0; col < 5; col++){
		int x = digitalRead(N_C[col]);
		if(x != pb_matrix[row][col]){
			pb_matrix[row][col] = x;
			Serial.print(messages[row][col]); Serial.println(pb_matrix[row][col]);
		}
	}

	pinMode(N_R[row], INPUT_PULLUP);
	delay(1);
}


void loop()
{

		//static bool connectionState = false;
		static bool connectionState = false;

		if (!connectionState)
		{
				Serial.println(controllerIdentifier);
				delay(500);

				while (Serial.available())
				{
						String receivedData = Serial.readStringUntil('\n');
						if (receivedData == startCommand)
						{
								connectionState = true;
								Serial.println("Connection established.");
								break;
						}
				}
		}
		else
		{
		/*
				pinMode(r2, OUTPUT);
				digitalWrite(r2, 0);
				read_selector(2);

				pinMode(r2, INPUT);
				pinMode(r1, OUTPUT);
				digitalWrite(r2, 1);
				digitalWrite(r1, 0);
				read_selector(1);
				pinMode(r1, INPUT);
				pinMode(r0, OUTPUT);
				digitalWrite(r1, 1);
				digitalWrite(r0, 0);
				read_selector(0);
				pinMode(r0, INPUT);
				digitalWrite(r0, 1);
*/
				for (int row = 0; row < 4; row++){
					read_pb_row(row);
				}

		}
}
