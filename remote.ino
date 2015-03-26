#include <SoftwareSerial.h>
#include <XBee.h>

SoftwareSerial mySerial(16, A1);

int buttons[] = {12, 11, 10, 9, 5, 6, 7, 8, 13};
int state[9];

int dataPin = 4;
int shiftClk = 3;
int storeClk = 2;

int color = 0;

int progState[8];
int prevState[8];

long rgb = 0L;
int rgbMap[][3] = {
	{16, 18, 17},
	{22, 8, 23},
	{12, 14, 13},
	{2, 4, 3},
	{21, 19, 20},
	{11, 9, 10},
	{1, 15, 0},
	{7, 5, 6}
};

XBee xbee = XBee();
Rx16Response rx16 = Rx16Response();

void setup() {
	Serial.begin(9600);
	mySerial.begin(9600);
	xbee.setSerial(mySerial);

	for (int i = 0; i < 9; i++) {
		pinMode(buttons[i], INPUT);
	}

	pinMode(dataPin, OUTPUT);
	pinMode(shiftClk, OUTPUT);
	pinMode(storeClk, OUTPUT);
}

void loop() {
	xbee.readPacket();
	if (xbee.getResponse().isAvailable()) {
		if (xbee.getResponse().getApiId() == RX_16_RESPONSE) {
			xbee.getResponse().getRx16Response(rx16);

			int pktId = rx16.getData(0);
			if (pktId == 2) { // Change preview
				bitWrite(rgb, rgbMap[rx16.getData(1)][1], rx16.getData(2));
			} else if (pktId == 3) { // Change program
				bitWrite(rgb, rgbMap[rx16.getData(1)][0], rx16.getData(2));
			}
		}
	}

	for (int i = 0; i < 9; i++) {
		int newValue = digitalRead(buttons[i]);
		if (newValue != state[i]) {
			state[i] = newValue;
			if (state[i] == 1) {
				if (i == 8) {
					uint8_t payload[] = { 1 };
					Tx16Request tx = Tx16Request(0x0, payload, sizeof(payload));
					xbee.send(tx);
				} else {
  					uint8_t payload[] = { 2, i };
					Tx16Request tx = Tx16Request(0x0, payload, sizeof(payload));
					xbee.send(tx);
				}
			}
		}
	}

	digitalWrite(shiftClk, HIGH);
	if ((++color % 24) == 0) {
		digitalWrite(storeClk, HIGH);
	}
	delay(1);
	digitalWrite(shiftClk, LOW);
	digitalWrite(storeClk, LOW);
	digitalWrite(dataPin, bitRead(rgb, color % 24) ? LOW : HIGH);
	delay(1);
}
