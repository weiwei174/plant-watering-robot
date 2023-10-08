
#include "PC_FileIO.c"

const float CM_TO_DEG = 180/(4*PI);
const int MIN_POW = 10;
const int STD_SPEED = -40;
const int TRN_SPEED = 15;
const int TRN_ANG = 88;
const int FULL_TURN = 176;


// fileIO
void readFile(TFileHandle & fin, int numofPlant, int numColours, int * xcoords, int * ycoords,
							int * colourIndex, int * waterAmt)
{
	for (int i = 0; i < numofPlant; i++)
  {
  // explicit type conversion to avoid error where compiler takes in xcoords[i] as "long" type
		int x = 0, y = 0;
    readIntPC(fin, x);
    readIntPC(fin, y);
    xcoords[i] = x;
    ycoords[i] = y;
  }

  for (int i =0; i< numColours; i++)
  {
 		int index = 0, water = 0;
    readIntPC(fin, index);
    readIntPC(fin, water);
    colourIndex[i] = index;
    waterAmt[i] = water;
  }
}

//sensor functions

void configureAllSensors()
{
	SensorType[S1] = sensorEV3_Touch;
	SensorType[S2] = sensorEV3_Color;
	wait1Msec(50);
	SensorMode[S2] = modeEV3Color_Color;
	wait1Msec(50);
	SensorType[S3] = sensorEV3_Color;
	wait1Msec(50);
	SensorMode[S3] = modeEV3Color_Color;
	wait1Msec(50);
	SensorType[S4] = sensorEV3_Gyro;
	wait1Msec(50);
	SensorMode[S4] = modeEV3Gyro_Calibration;
	wait1Msec(100);
	SensorMode[S4] = modeEV3Gyro_RateAndAngle;
	wait1Msec(50);
}

void checkTouch(bool & fails)
{
	if(SensorValue[S1] == 1)
	{
		fails = true;
		eraseDisplay();
		displayString(5, "Collision Detected. Error");
	}
}

//turning functions

//direct parameter is +1 or -1 to indicate direction
void turnCorrection(int pwr, int direct, float angle)
{
	//first bit decreases with time, MIN_POW is a constant minimum
	motor[motorA] = (pwr * (1 - (getGyroDegrees(S4) / angle)) + MIN_POW) * direct;
	motor[motorD] = -(pwr * (1 - (getGyroDegrees(S4) / angle)) + MIN_POW) * direct;
}

void driveBoth(int pow1, int pow2)
{
  motor[motorA] = pow1;
  // callibration as motorD seems to start up before motor A, causing inconsistent turning
	wait1Msec(10);
  motor[motorD] = pow2;
}

void drive(int pwr)
{
  motor[motorA] = pwr - 1;
  // callibration as motorD seems to start up before motor A, causing tilt
  if (pwr != 0)
  	wait1Msec(10);
  motor[motorD] = pwr;
}

void turnAngle(float angle, bool & fails)
{
	//standard turning function (if statements to turn in different directions)
	int direct = 1; // direction
	resetGyro(S4);

	if (angle>0)
		driveBoth(TRN_SPEED,-TRN_SPEED);
	else
	{
		driveBoth(-TRN_SPEED,TRN_SPEED);
		direct *= -1;
	}

	while (abs(getGyroDegrees(S4))< (abs(angle)-2) && !fails)
	{
    turnCorrection(TRN_SPEED, direct, angle);
		checkTouch(fails);
  }

	drive(0);
	wait1Msec(1000);
}

//driving functions

void courseCorrection(int pwr)
{
	int degrees_off = getGyroDegrees(S4);

	if(degrees_off > 1) //if its too far right, weakens left motor
		motor[motorD] = pwr + 2;

	if(abs(degrees_off) < 1) //resets motpow
		motor[motorA] =	motor[motorD] = pwr;

	if(degrees_off < -1) //if its too far left, weakens right motor
		motor[motorA] = pwr + 2;
}

void driveDist(int dist, int pwr, bool & fails)
{
  //standard driving function
	resetGyro(S4);
	nMotorEncoder[motorA]=0;
	drive(pwr);
	resetGyro(S4);

	while (abs(nMotorEncoder[motorA]) < (dist*CM_TO_DEG) && !fails)
	{
    courseCorrection(pwr);
		checkTouch(fails);
  }

	drive(0);
	wait1Msec(1000);

}

// higher-level drive functions

void relPos(int * x_coord, int * y_coord, int * rel_x_pos, int * rel_y_pos, int size)
{
	rel_x_pos[0] = x_coord[0];
	rel_y_pos[0] = y_coord[0];

	for (int index = 1; index < size; index++)
	{
		rel_x_pos[index] = x_coord[index] - x_coord[index - 1];
		rel_y_pos[index] = y_coord[index] - y_coord[index - 1];
	}
}



void goToCoords(int x, int y, bool & fails)
{
  int direction = 1;

  //turns 180 to face negative y, changes direction variable to account for turn
  if(y < 0)
  {
  	turnAngle(FULL_TURN, fails);
    direction *= -1;
    wait1Msec(1000);
  }
  if(x < 0) //changes direction variable to account for turn
    direction *= -1;

  //drives along x
  turnAngle(TRN_ANG * direction, fails);
	wait1Msec(1000); // waits are added to improve accuracy
  driveDist(abs(x), STD_SPEED, fails);

  //drives along y
  wait1Msec(1000);
  turnAngle(-TRN_ANG * direction, fails);
	wait1Msec(1000);
  driveDist(abs(y), STD_SPEED, fails);
}


//gui
void guiInput(int * x, int * y, int & plantCount)
{
	//asks for number of inputs, max 4
  eraseDisplay();
	displayString(3, "Choose how many plants you have");

  plantCount = 1;
  int cor_in = false;

	//standard display
  displayString(4, "Change number of plants");
	displayString(5, "using up/down buttons");
	displayString(6, "Press enter when ready");
  do{
		//allows you to scroll through and set plantcount, max 4 min 1
		displayString(8, "num = %d", plantCount);

		while(!getButtonPress(buttonAny))
		{}
		if(getButtonPress(buttonUp) && plantCount < 4)
			plantCount++;
		else if(getButtonPress(buttonDown) && plantCount > 1)
			plantCount--;
		else if(getButtonPress(buttonEnter))
			cor_in = true;
		while(getButtonPress(buttonAny))
		{}
	} while(!cor_in);

  for(int index = 0; index < plantCount; index++)
  {
    //one iteration per set of coords
		int input = 0;
    string coord = "x";

  	for(int i = 0; i < 2; i++)
  	{
  		//one iteration per coord, x first
			eraseDisplay();
      displayString(4, "Change %s%d value using", coord, index+1);
  		displayString(5, "up/down buttons");
  		displayString(6, "Press enter when ready");
      cor_in = false;
      input = 0;

  		do{
  			displayString(8, "%s = %d", coord, input);
  			//same sort of setup as for plantcount input, but max 300, min -300
  			while(!getButtonPress(buttonAny))
  			{}
  			if(getButtonPress(buttonUp) && input < 301)
  				input += 10;
  			else if(getButtonPress(buttonDown) && input > -301)
  				input -= 10;
  			else if(getButtonPress(buttonEnter))
  				cor_in = true;
  			while(getButtonPress(buttonAny))
  			{}
  		} while(!cor_in);

  		//this part saves inputted value then changes displayed value
  		if(i == 0)
  			x[index] = input;
  		else
  			y[index] = input;

  		coord = "y";
  	}
  }
}

bool guiStart()
{
	int method = 0;
  while(method == 0)
	{
		//display
		eraseDisplay();
		displayString(2, "Choose your location input :");
		displayString(3, "method:");
		displayString(7, "Left Button: Manual Input");
		displayString(9, "Right Button: File Input");

		//waits for input. If correct, then exits.
		//If incorrect, displays incorrect input then goes throug loop again
		while(!getButtonPress(buttonAny))
		{}
		eraseDisplay();
		if(getButtonPress(buttonLeft))
			method = 1;
		else if(getButtonPress(buttonRight))
			method = 2;
		else
		{
      displayString(5, "Invalid Input");
      while(getButtonPress(buttonAny))
		  {}
		  wait1Msec(1000);
    }
	}
	if(method == 1)
		return false; // use gui

	else
		return true; // use file
}

//plant functions
int plantType (int numCol, int * colour, int * ml)
{
	const int ML2MOTOR = 87;	//Unit conversion factor found through testing
	const int INVALID = -1;

  int colourSens = SensorValue[S3];

  for(int index = 0; index < numCol; index++)
  	if(colourSens == colour[index])
  		return ml[index] * ML2MOTOR;

  return INVALID;
}

void pumpWater (int motRot)
{
	const int PUMPSPEED = 50;
	const int INVALID = -1;
	nMotorEncoder[motorC] = 0;

	if (motRot != INVALID)
	{
 		motor[motorC] = PUMPSPEED;
 		while(nMotorEncoder[motorC] <= motRot)
 		{
 			motor[motorA] = motor[motorD] = 0; //Use to fix random movement while pumping
 		}
 		motor[motorC] = -PUMPSPEED;
 		wait1Msec(3000);
 		motor[motorC] = 0;
 		wait1Msec(5000);
 	}
	else
		wait1Msec(2000);
}




//TASK MAIN
task main
{
	configureAllSensors();

	TFileHandle fin;
  bool fileOkay = openReadPC(fin, "plantinfo.txt");
  if (!fileOkay)
		displayString(3, "Error opening file.");

  const int MAX_PLANT = 10; // max number of plants to determine array size before actual number is read in
  const int MAX_COL = 5; // max number of colours before actual number is read in

	int x[MAX_PLANT];
  int y[MAX_PLANT];
  int relx[MAX_PLANT];
  int rely[MAX_PLANT];
  int colourIndex[MAX_COL]; // array of allowed colours that identify plant types
  int waterAmt[MAX_COL]; // corresponding water quantities

	int plantCount = 0, numColour = 0; // will store actual number of plants and colours, read in from file

  if(guiStart()) // read in from file
  {
    readIntPC(fin, plantCount);
    readIntPC(fin, numColour);
    readFile(fin, plantCount, numColour, x, y, colourIndex, waterAmt);
  }
  else // manual input
  {
    guiInput(x, y, plantCount);
    // default colour and water quantities since file is not read in
    numColour = 4;
    colourIndex[0] = 2;
    colourIndex[1] = 3;
    colourIndex[2] = 5;
    colourIndex[3] = 6;
    waterAmt[0] = 100;
    waterAmt[1] = 100;
    waterAmt[2] = 50;
    waterAmt[3] = 10;
  }

  relPos(x, y, relx, rely, plantCount);

  bool fails = false;

  //wait for button to start up
  while (!getButtonPress(buttonEnter))
	{}
	while (getButtonPress(buttonEnter))
	{}
	wait1Msec(500);

  int index = 0;
  // loop while collision not detected and water level is not low
  //sensor S2 detects water level based on floating red strip
  for(index = 0; index < plantCount && !fails &&
      SensorValue[S2] != (int)colorRed; index++)
  {
    eraseDisplay();
  	displayString(5, "Plant %d", (index + 1));
  	resetGyro(S4);
    goToCoords(relx[index], rely[index], fails);
    if(!fails)
    {
    	wait1Msec(3000);
      pumpWater(plantType(numColour, colourIndex, waterAmt));
    }
    // if facing backwards, must rotate 180 before proceeding since locations are based on forward-facing coordinate system
    if (rely[index] < 0)
    	turnAngle(FULL_TURN, fails);
  }

  if(SensorValue[S2] == (int)colorRed)
  	displayString(5, "Water Low");

  if (index > 0) // only return to original location if robot moved from start location
  {
  	goToCoords((-x[index - 1]), (-y[index - 1]), fails);
    turnAngle(FULL_TURN, fails);
  }

  wait1Msec(10000);
  closeFilePC(fin);
}
