Alex Belliard and Joey Cerulli
ECE-218 - Embedded Microcontroller Projects
Instructor: Cherrice Traver
3/10/26

Final Project - BC Specialz Audio System

System Behavior
Our system, the BC Specialz, feature a pair of headphones that allow you to control volume via a knob, skip, play, pause or 
reverse your song choice via 3 buttons, and an LCD screen that displays text of the song title and artist that's currently
playing. The text on the LCD will scroll by if the title or artist is too big to fit on the screen, otherwise it will stay stationary.


Design Alternatives


╔══════════════════════════════════════════════════════════════════════════════════════════════════════════════════╗
║                                                LCD Subsystem                                                     ║
╠═════════════════════════════════╦══════════════════════════════════════════╦═════════════════════════════════════╣
║          Specification          ║               Test Process               ║               Results               ║
╠═════════════════════════════════╬══════════════════════════════════════════╬═════════════════════════════════════╣
║ Enable engine start (i.e.,      ║ 4 buttons:                               ║ All tests passed.                   ║
║ light the green LED) while      ║ DRIVER_OCC                               ║ 1.Green light on                    ║
║ both seats are occupied         ║ PASS_OCC                                 ║ 2.“Passenger seatbelt not fastened” ║
║ (DRIVER_OCC, PASS_OCC)          ║ DRIVER_BELT                              ║ 3. All error messages printed.      ║
║ and seatbelts fastened          ║ PASS_BELT                                ║                                     ║
║ (DRIVER_BELT, PASS_BELT).       ║ 1. All buttons pressed                   ║                                     ║
║ Otherwise print appropriate     ║ 2. All but one button (PASS_OCC) pressed ║                                     ║
║ error messages.                 ║ 3. No buttons pressed                    ║                                     ║
╠═════════════════════════════════╬══════════════════════════════════════════╬═════════════════════════════════════╣
║ Start the engine (i.e., light   ║ All buttons pressed, green               ║ Test passed.                        ║
║ the yellow LED, turn off Green) ║ LED is on, ignition button               ║ Engine light illuminates and        ║
║ when ignition is enabled (green ║ pressed                                  ║ stays on after ignition button      ║
║ LED) and ignition button is     ║                                          ║ is released.                        ║
║ pressed  (i.e., before the      ║                                          ║                                     ║
║ button is released).            ║                                          ║                                     ║
╠═════════════════════════════════╬══════════════════════════════════════════╬═════════════════════════════════════╣
║ Test ignition turning off       ║ Set all requirements and turn on the     ║ Test passed.                        ║
║ the car.                        ║ car. Click the ignition button again     ║ When the ignition was pressed       ║
║                                 ║ and check if the engine light turns off. ║ again, the engine light shuts off.  ║
╠═════════════════════════════════╬══════════════════════════════════════════╬═════════════════════════════════════╣
║ Test that engine stays on while ║ Set all requirements and turn on the     ║ Test passed.                        ║
║ requirements are changed after  ║ car, so that the engine light is         ║ When requirements were altered      ║
║ the car is already on.          ║ illuminated. Then change some of the     ║ after the engine button was         ║
║                                 ║ requirements and make sure the engine    ║ already on, the light stayed on.    ║
║                                 ║ button stays illuminated.                ║                                     ║
╠═════════════════════════════════╩══════════════════════════════════════════╩═════════════════════════════════════╣
║                                                Bluetooth subsystem                                               ║
╠═════════════════════════════════╦══════════════════════════════════════════╦═════════════════════════════════════╣
║ Only works while the engine     ║ - Turn on car and test that wipers       ║ All tests passed.                   ║
║ is running.                     ║ are fully functional.                    ║ - When car is on wipers will        ║
║                                 ║                                          ║ turn on after potentiometer is      ║
║                                 ║                                          ║ turned.                             ║
║                                 ║                                          ║                                     ║
║                                 ║ - Turn car off and make sure wipers      ║ - When the wipers are running and   ║
║                                 ║ are not able to turn on or change modes. ║ the carturns off, wipers turn off   ║
╠═════════════════════════════════╬══════════════════════════════════════════╬═════════════════════════════════════╣
║ Test off                        ║ - When the potentiometer reads the       ║ All tests passed.                   ║
║                                 ║ correct value, the wipers should not     ║ - When the potentiometer is spun    ║
║                                 ║ spin                                     ║ the wipers turn off                 ║
║                                 ║                                          ║                                     ║
║                                 ║ - When off, Mode will print but interval ║ - When potentiometer is spun the    ║
║                                 ║ will be empty                            ║ Mode switches to OFF and the        ║
║                                 ║                                          ║ Interval is empty                   ║
╠═════════════════════════════════╬══════════════════════════════════════════╬═════════════════════════════════════╣
║ Test high                       ║ - Turn potentiometer to high and check   ║ All tests passed.                   ║
║                                 ║ that the wipers run at 25 rpm            ║ - When potentiometer is high the    ║
║                                 ║                                          ║ wipers run at 25 rpm, or 1.5 degrees║
║                                 ║                                          ║ per second                          ║
║                                 ║ - Turn potentiometer to high and check   ║ - When potentiometer is high        ║
║                                 ║ that the wipers run at a 90 degree angle ║ the wipers go 90 degrees both ways  ║
╠═════════════════════════════════╬══════════════════════════════════════════╬═════════════════════════════════════╣
║ Test low                        ║ - Turn potentiometer to low and check    ║ All tests passed.                   ║
║                                 ║ that the wipers run at 10 rpm.           ║ - When potentiometer is low the     ║
║                                 ║                                          ║ wipers run at 10 rpm, 1 degree per  ║
║                                 ║                                          ║ second.                             ║
║                                 ║ - Turn potentiometer to low and check    ║ - When potentiometer is low         ║
║                                 ║ that the wipers run at a 90 degree angle ║ the wipers go 90 degrees both ways  ║
╠═════════════════════════════════╬══════════════════════════════════════════╬═════════════════════════════════════╣
║ Test interval: short            ║ - Turn potentiometer to interval slow    ║ All tests passed.                   ║
║                                 ║ mode and check that the interval between ║ - When potentiometer is interval and║
║                                 ║ wipes is 1 second                        ║ slow mode, the motor waits 1 second ║
║                                 ║                                          ║ before wiping again                 ║
║                                 ║ - Turn potentiometer to int. short and   ║ - When potentiometer is int. short  ║
║                                 ║ check that the wipers run at a 90 degree ║ the wipers go 90 degrees both ways  ║
║                                 ║ angle.                                   ║                                     ║
╠═════════════════════════════════╬══════════════════════════════════════════╬═════════════════════════════════════╣
║ Test interval: med              ║ - Turn potentiometer to interval medium  ║ All tests passed.                   ║
║                                 ║ mode and check that the interval between ║ - When potentiometer is interval and║
║                                 ║ wipes is 3 seconds                       ║ med mode, the motor waits 3 seconds ║
║                                 ║                                          ║ before wiping again                 ║
║                                 ║ - Turn potentiometer to int. med and     ║ - When potentiometer is int. med the║
║                                 ║ check that the wipers run at a 90 degree ║ wipers go 90 degrees both ways      ║
║                                 ║ angle.                                   ║                                     ║
╠═════════════════════════════════╬══════════════════════════════════════════╬═════════════════════════════════════╣
║ Test interval: high             ║ - Turn potentiometer to interval high    ║ All tests passed.                   ║
║                                 ║ mode and check that the interval between ║ - When potentiometer is interval and║
║                                 ║  wipes is 5 seconds                      ║ high mode, the motor waits 3 seconds║
║                                 ║                                          ║ before wiping again                 ║
║                                 ║ - Turn potentiometer to int. high and    ║ - When potentiometer is int. high   ║
║                                 ║ check that the wipers run at a 90 degree ║ the wipers go 90 degrees both ways  ║
║                                 ║ angle.                                   ║                                     ║
╠═════════════════════════════════╩══════════════════════════════════════════╩═════════════════════════════════════╣
║                                                Buttons subsystem                                                 ║
╠═════════════════════════════════╦══════════════════════════════════════════╦═════════════════════════════════════╣
║ Only works while the engine     ║ - Turn on car and test that wipers       ║ All tests passed.                   ║
╠═════════════════════════════════╩══════════════════════════════════════════╩═════════════════════════════════════╣
║                                                Volume subsystem                                                  ║
╠═════════════════════════════════╦══════════════════════════════════════════╦═════════════════════════════════════╣
║ Only works while the engine     ║ - Turn on car and test that wipers       ║ All tests passed.                   ║