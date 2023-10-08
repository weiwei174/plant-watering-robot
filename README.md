# plant-watering-robot
Autonomous plant-watering robot coded in RobotC, a language based in C++ used to interact with the Lego EV3 robot brick.

## Purpose

The goal of this project was to design and build an automatic robot that can navigate to multiple locations of the room and ouput varying amounts of water to help busy or mobility-restricted plant owners take care of their plants.

## Key functions

To improve the robot's performance and convenience for users, the software includes the following functions:

**Multiple location input methods:**

Users can specify plant coordinates in cm from the robot's starting location using the file uploaded to the robot or manually input coordinates via the Lego EV3 robot block's GUI

**Plant type detection:**

Based on the colour labelled on a plant pot, the robot can detect the type of plant using a colour sensor and output the corresponding quantity of water to allow for varying water needs of plants

**Accuracy of movement:** 

To compensate for hardware imperfections (delayed motor startups, imperfect gyroscope readings, etc), PID-like controls are implemented for turning and thorough testing was done to implement adjustments for motor startup delay. This means:
- Decreasing the motor power of the robot during turning as a function of the difference betweent the current angle and the target angle
- Adding time delay for one of the motors based on measurements and testing to ensure both motors start at the same time
- Stopping motors early before target distance or angle is reached to prevent over-shooting

**Water level detection**

Using a floating colour bar in the robot's transparent water tank, the robot can detect when the water is low using a colour sensor mounted to the outside of the tank and alert users to refill the robot.

## Testing

Testing was done to ensure robot accuracy and performance at the function level and the global level. 
- Each function was tested separately to ensure the desired output was achieved through means such as displaying calculated values, measuring water output, or ensuring responsiveness to sensor signals and stimuli.
- Integration was tested iteratively and throughly to account for edge cases in navigation, including keeping track of the robot's heading, ensuring robot can reach all specified locations, and movement for all combinations of inputs can be calculated
- Extensive finetuning of controls and adjustments to movement and motor controls were performed to enhance accuracy and consistency of path tracking as much as possible provided the imperfections in the hardware we were tasked to use

  
