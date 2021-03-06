// EE10170 Coursework 2 C++ Code
// bi242@bath.ac.uk
// March 2022
// Main Source Code (Run This One!)

#include <iostream>
#include <string>
#include <cmath>
#include "sensors.cpp"
#include "logger.cpp"
using namespace std;

//SET THIS FLAG TO TRUE IF YOU WANT TO SEE THE DEBUGGING OUTPUTS - THIS IS SLOW!
bool debugging = false;

//LOCATIONS OF THE CSV FILES:
string sensor1location = "sensors/sensor_1.csv";
string sensor2location = "sensors/sensor_2.csv";
string sensor3location = "sensors/sensor_3.csv";
string verbose_location = "verbose.csv";
string motor_location = "RESULTS/motor_output.csv";
string A_location = "RESULTS/lp_a.csv";
string B_location = "RESULTS/lp_b.csv";
string C_location = "RESULTS/lp_c.csv";
string D_location = "RESULTS/lp_d.csv";
string E_location = "RESULTS/lp_e.csv";
string F_location = "RESULTS/lp_f.csv";

//// FUNCTIONS
float sensor_fusion(float sensor1, float sensor2, float sensor3){
    float top = (float)3*(float)(sensor1 - sensor3);
    float bottom = (float)sensor2;
    float fraction = float(top) / (float)bottom;
    //cout << "\n" << top << "/" << bottom << "=" << fraction << endl;
    float fused = fraction - 3;
    //cout << "*********Fusion process: (" << top << "/" << bottom << ")-3=" << fused << "\n";

    //Check that none of the outputs are NAN (number/0) or INF (0/number) in that case just cap them at -3 or 3
    if (isnan(fused) || isnan(-fused)){
        return -3;
    }
    if (isinf(fused)){
        return 3;
    }
    else{
        return fused;
    }
}
float clip(float value){
    if (value > 1){
        return 1;
    }
    else if(value < -1){
        return -1;
    }
    else{
        return value;
    }
}
string delim(string str, string delim){
    // Returns the stuff after the delimiter:
    return str.substr(str.find(delim)+1);
}
//// END FUNCTIONS

//// VARIBALE DEFINITIONS
float fused;
float clipped_motor;
float inverse_clipped;
float previousSensorValue = -1;
int counter = 1;
//// END VARIABLE DEFINITIONS

//Start Main Loop
int main() {
   if (not debugging){
        /*
        If we are debugging then we want cout otherwise we set it to a fail state, so it outputs nothing and makes the program quicker
        */
        std::cout.setstate(std::ios_base::failbit);
    }
   else {
        cout.clear();
    }

    // INSTANTIATE SOME OBJECTS!!!
    Sensor1 sensor1;
    Sensor2 sensor2;
    Sensor sensor3;
    Logger a, b, c, d, e, f, verbose, motors;

    // Open the sensor files in the objects
    sensor1.open(sensor1location);
    sensor2.open(sensor2location);
    sensor3.open(sensor3location);

    //Open the output file streams
    a.open(A_location); b.open(B_location); c.open(C_location); d.open(D_location); e.open(E_location);f.open(F_location);
    motors.open(motor_location);
    //Verbose is all of the 6 intermediate outputs as well as the final motor all in one file just for checking purposes
    verbose.open(verbose_location);

    // Make sure that all the sensors and outputs are open
    if(!sensor1.is_open()) throw runtime_error("Could not open Sensor 1");
    if(!sensor2.is_open()) throw runtime_error("Could not open Sensor 2");
    if(!sensor3.is_open()) throw runtime_error("Could not open Sensor 3");
    if(!verbose.is_open()) throw runtime_error("Could not open Verbose file");
    if(!motors.is_open()) throw runtime_error("Could not open Motors file");
    if(!a.is_open()) throw runtime_error("Could not open log point A");
    if(!b.is_open()) throw runtime_error("Could not open log point B");
    if(!c.is_open()) throw runtime_error("Could not open log point C");
    if(!d.is_open()) throw runtime_error("Could not open log point D");
    if(!e.is_open()) throw runtime_error("Could not open log point E");
    if(!f.is_open()) throw runtime_error("Could not open log point F");

    //Set up the output files:
    verbose << "line,a,b,c,d,e,f,motorA,motorB\n";
    motors << "motor,,\n----------------,,\nLine No,Motor A,Motor B\n";
    a<<"Log A\n"; b<<"Log B\n"; c<<"Log C\n"; d<<"Log D\n"; e<<"Log E\n"; f<<"Log F\n";

    //While there's still a line left to read in the sensor1file:
    while (getline(sensor1, sensor1.stringvalue)){
        //Get the other two string lines of each of the other two sensor and save to sensorXvalue variable
        getline(sensor2, sensor2.stringvalue);
        getline(sensor3, sensor3.stringvalue);

        //Delimit those strings, so they contain just the sensor values and not the line numbers:
        sensor1.stringvalue = delim(sensor1.stringvalue, ",");
        sensor2.stringvalue = delim(sensor2.stringvalue, ",");
        sensor3.stringvalue = delim(sensor3.stringvalue, ",");

        //We are done delim-ing using string, so we convert to a float, so we can perform math on it.
        sensor1.floatvalue = stof(sensor1.stringvalue);
        sensor2.floatvalue = stof(sensor2.stringvalue);
        sensor3.floatvalue = stof(sensor3.stringvalue);

        //Print the unedited string sensor values:
        cout << "Sensor1original: " << sensor1.stringvalue << endl << "Sensor2original: " << sensor2.stringvalue << endl << "Sensor3original: " << sensor3.stringvalue << "\n";

        //convert the first sensor value using the function
        sensor1.floatvalue_converted = sensor1.convert(sensor1.floatvalue);
        cout << "Sensor1converted: (2/3)*sqrt(" << sensor1.floatvalue <<")= " << sensor1.floatvalue_converted << endl;
        //Output the result to the verbose (POINT A)
        verbose << counter << "," << sensor1.floatvalue_converted << ","; a << sensor1.floatvalue_converted << "\n";

        //Convert the second sensor using the previous value and the current one.
        sensor2.floatvalue_converted = sensor2.convert(previousSensorValue, sensor2.floatvalue);
        cout << "Sensor2converted: " << sensor2.floatvalue << "-" << previousSensorValue << "= " << sensor2.floatvalue_converted << endl;
        //Output the result to the verbose for (POINT B)
        verbose << sensor2.floatvalue_converted << ","; b << sensor2.floatvalue_converted << "\n";

        //Update the previous sensor value as the raw sensor 2 value for the next time the loop runs
        previousSensorValue = sensor2.floatvalue;

        //Scale all the sensor using their respective biases
        sensor1.floatvalue_converted_scaled = sensor1.scale(sensor1.floatvalue_converted, 2.7, 1);
        sensor2.floatvalue_converted_scaled = sensor2.scale(sensor2.floatvalue_converted, 0.7, -0.5);
        sensor3.floatvalue_converted_scaled = sensor3.scale(sensor3.floatvalue, 1, 0.2);
        //^^Remember that sensor3floatvalue_converted doesn't exist, so we use raw sensor3.floatvalue here (not the .floatvalue_converted, because it doesnt exist)

        //Output the scaled values to the verbose file and the respective Logging Points
        verbose << sensor1.floatvalue_converted_scaled << ","; c << sensor1.floatvalue_converted_scaled << "\n";     // (POINT C)
        verbose << sensor2.floatvalue_converted_scaled << ","; d << sensor2.floatvalue_converted_scaled << "\n";     // (POINT D)
        verbose << sensor3.floatvalue_converted_scaled << ","; e << sensor3.floatvalue_converted_scaled << "\n";     // (POINT E)

        cout << "Sensor1scaled: 2.7(" << sensor1.floatvalue_converted << "-1)= " << sensor1.floatvalue_converted_scaled << endl << "Sensor2scaled: 0.7(" << sensor2.floatvalue_converted << "--0.5)= " << sensor2.floatvalue_converted_scaled << endl << "Sensor3scaled: 1(" << sensor3.floatvalue << "-0.2)= " << sensor3.floatvalue_converted_scaled << "\n";

        //Sensor fusion time!
        fused = sensor_fusion(sensor1.floatvalue_converted_scaled, sensor2.floatvalue_converted_scaled, sensor3.floatvalue_converted_scaled);
        //Output that to the console
        cout << "Fused: ((3(" << sensor1.floatvalue_converted_scaled << "-" << sensor3.floatvalue_converted_scaled << "))/" << sensor2.floatvalue_converted_scaled << ")-3= " << fused << endl;
        //Send to verbose and Logging Poing (POINT F)
        verbose << fused << ","; f << fused << "\n";

        //Output the clipped motor value to the motor.csv and the verbose files
        clipped_motor = clip(fused);
        inverse_clipped = clipped_motor * -1;
        motors << counter << "," << clipped_motor <<","<< inverse_clipped << endl; verbose << clipped_motor <<","<< inverse_clipped << endl;
        cout << "Original was " << fused << " and Clipped: " << clipped_motor << " " << "Inverse (" << clipped_motor << "*-1=):  " << inverse_clipped << endl;

        cout << "\n\n\n";
        //Increase the line counter each time the loop runs
        counter += 1;
    }

    // Close all the files because we don't need them anymore
    sensor1.close(); sensor2.close(); sensor3.close();
    verbose.close(); motors.close(); 
    a.close(); b.close(); c.close(); d.close(); e.close(); f.close();

    //Clear the cout and close the main
    cout.clear();
    cout << "All done! Closing!";
    return 0;
}