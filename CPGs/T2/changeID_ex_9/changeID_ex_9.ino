#include <actuator.h>
#include <Dynamixel2Arduino.h>

#include <DynamixelSDK.h>
//#include <iostream>
#include <cmath>

#define ADDR_AX_ID           3                 
#define ADDR_AX_MAX_TORQUE              34 
#define ADDR_AX_TORQUE_ENABLE           24                 // Control table address is different in Dynamixel model
#define ADDR_AX_GOAL_POSITION           30
#define ADDR_AX_PRESENT_POSITION        36
#define ADDR_AX_MOVING_SPEED            32
#define MOVING                          46

// Protocol version
#define PROTOCOL_VERSION                1.0                 // See which protocol version is used in the Dynamixel


// Default setting
#define DXL_ID                          1                   // Dynamixel ID: 1
#define DXL_NEW_ID                      1                   // Dynamixel ID: 2


#define BAUDRATE                        1000000
#define DEVICENAME                      "1"                 //DEVICENAME "1" -> Serial1(OpenCM9.04 DXL TTL Ports)
                                                            //DEVICENAME "2" -> Serial2
                                                            //DEVICENAME "3" -> Serial3(OpenCM 485 EXP)
#define TORQUE_ENABLE                   1                   // Value for enabling the torque
#define TORQUE_DISABLE                  0                   // Value for disabling the torque
#define DXL_MINIMUM_POSITION_VALUE      0                 // Dynamixel will rotate between this value
#define DXL_MAXIMUM_POSITION_VALUE      1000                 // and this value (note that the Dynamixel would not move when the position value is out of movable range. Check e-manual about the range of the Dynamixel you use.)
#define DXL_MOVING_STATUS_THRESHOLD     20                  // Dynamixel moving status threshold

#define ESC_ASCII_VALUE                 0x1b

#define mean                            500.0
#define amplitude                       300.0
#define frequency                       0.5
#define DISCRIPTION_LENGTH     15
#define NOMBER_RS_NEURONS     1

// Create PortHandler instance
dynamixel::PortHandler *portHandler;

// Create PacketHandler instance
dynamixel::PacketHandler *packetHandler;

//***********Set Global Variables****************
int goalPosition = 0;
int isMoving = 0;
int dxl_comm_result = COMM_TX_FAIL;             // Communication result
uint8_t dxl_error = 0;                          // Dynamixel error
int16_t dxl_present_position = 0;               // Present position

uint8_t dxl_new_id = DXL_NEW_ID; 
unsigned int mydelay = 10; // ms

/******************/ 
//struct RSneuron 
/******************/ 
struct RSneuron { 
   char discription[DISCRIPTION_LENGTH]; // name 
   double tao_m   = 0.1;  
   double tao_s   = 20*tao_m; 
   double Af      = 5; 
   double Es      = 0; 
   double sigma_s = 0; 
   double sigma_f = 0; 
   double V_0     = 0;
   double q_0     = 0;
   double V       = V_0;
   double q       = q_0; 
   double inj_cur = 0; 
   double inj_cur_MultiplicationFactor = 1; 
   
} rs_neuron[NOMBER_RS_NEURONS]; 

//struct Pattern 
/******************/ 
struct Pattern{
  double sigma_s;
  double sigma_f;
  double tao_m;
  double InjCurrentMultiplicationFactor;
  };
/******************/ 

/*
  OSCILLATORY
   ^
   |     
   |   /\    /\    /\
   |  /  \  /  \  /  \
   | /    \/    \/    \
   ------------------------> 
*/
Pattern OSCILLATORY={4.6,1.5,0.1,1};

/******************/ 
inline double fun_v ( double V , double q , double Sf , double inj , double tm , double Af )
{ return  (double)(-1/tm)*( V-Af*tanh(Sf/Af*V)+q-inj) ; } 
/******************/ 
inline double  fun_q ( double V , double q , double ts , double Es , double Ss )
{ return (double)(1/ts)(-q+Ss*(V-Es)); } 
/******************/ 
void update_RS_neuron(struct RSneuron* rs_n)
{
int n = 20; 
//double x0 = t-ts;
//double xf = t;
double h; 
//h = (xf-x0)/(double)n;
h= ((double)mydelay/1000)/n;
double xa[20],ya[20],yb[20]; 
double k1,k2,k3,k4,k, l1,l2,l3,l4,l;  
//xa[0] = x0;
ya[0] = rs_n->V;
yb[0] = rs_n->q;

for (int i=0; i<n-1 ; i++)
{
  k1= h*fun_v( ya[i] , yb[i] , rs_n->sigma_f , rs_n->inj_cur*rs_n->inj_cur_MultiplicationFactor ,  rs_n->tao_m , rs_n->Af ); 
  l1 = h*fun_q( ya[i] , yb[i] , rs_n->tao_s , rs_n->Es , rs_n->sigma_s ); 

  k2= h*fun_v( ya[i]+k1/2 , yb[i]+l1/2 , rs_n->sigma_f , rs_n->inj_cur*rs_n->inj_cur_MultiplicationFactor , rs_n->tao_m , rs_n->Af ); 
  l2 = h*fun_q( ya[i]+k1/2 , yb[i]+l1/2 , rs_n->tao_s , rs_n->Es , rs_n->sigma_s);   
  
  k3= h*fun_v( ya[i]+k2/2 , yb[i]+l2/2 , rs_n->sigma_f , rs_n->inj_cur*rs_n->inj_cur_MultiplicationFactor , rs_n->tao_m , rs_n->Af ); 
  l3 = h*fun_q( ya[i]+k2/2 , yb[i]+l2/2 , rs_n->tao_s , rs_n->Es , rs_n->sigma_s );   

  k4= h*fun_v( ya[i]+k3 , yb[i]+l3 , rs_n->sigma_f , rs_n->inj_cur*rs_n->inj_cur_MultiplicationFactor , rs_n->tao_m , rs_n->Af ); 
  l4 = h*fun_q( ya[i]+k3 , yb[i]+l3 , rs_n->tao_s , rs_n->Es , rs_n->sigma_s );   
  
  k = 1/6.0 * (k1 + 2*k2 + 2*k3 + k4);
  l  = 1/6.0 * ( l1 + 2*l2  + 2*l3  +  l4);
 
  ya[i+1]= ya[i]+k;
  yb[i+1]= yb[i]+l ;
  //xa[i+1]= xa[i]+h;
} 
rs_n->V = ya[n-1]; 
rs_n->q = yb[n-1]; 

return; 
}
/******************/ 
void setup_RS_neurons(struct RSneuron* rs_n,struct Pattern myP, String str)
{
for(int i=0 ; i<(sizeof(str) / sizeof(str[0])) ; i++) 
     rs_n->discription[i] = str[i]; 

rs_n->tao_m = myP.tao_m; 
rs_n->tao_s = 20 * myP.tao_m; 
rs_n->sigma_s = myP.sigma_s;
rs_n->sigma_f = myP.sigma_f;
rs_n->inj_cur_MultiplicationFactor = myP.InjCurrentMultiplicationFactor;

}
/******************/ 
void update_locomotion_network(void)
{
for (int i = 0; i< NOMBER_RS_NEURONS ; i++)
  update_RS_neuron(&rs_neuron[i]);
}

void setup() {
  /* set the configuration of the RS neuron to match a desired output: OSCILLATORY, QUIESCENT, PLATEAU, ALMOSTOSC */
  setup_RS_neurons(&rs_neuron[0],OSCILLATORY,"First neuron");
  // put your setup code here, to run once:
  Serial.begin(115200);
  while(!Serial.available());
String a;
  if(Serial.available())
  {
    a= Serial.readString();// read the incoming data as string
    Serial.println(a); 
    dxl_new_id = a.toInt();
    delay(2000);
  }

  // Initialize portHandler. Set the port path
  // Get methods and members of PortHandlerLinux or PortHandlerWindows
  portHandler = dynamixel::PortHandler::getPortHandler(DEVICENAME);

  // Initialize packetHandler. Set the protocol version
  // Get methods and members of Protocol1PacketHandler or Protocol2PacketHandler
  packetHandler= dynamixel::PacketHandler::getPacketHandler(PROTOCOL_VERSION);

  if (portHandler->openPort())
  {
    Serial.print("Succeeded to open the port!\n");
  }
  else
  {
    Serial.print("Failed to open the port!\n");
    Serial.print("Press any key to terminate...\n");
    return;
  }

  // Set port baudrate
  if (portHandler->setBaudRate(BAUDRATE))
  {
    Serial.print("Succeeded to change the baudrate!\n");
  }
  else
  {
    Serial.print("Failed to change the baudrate!\n");
    Serial.print("Press any key to terminate...\n");
    return;
  }


  // Change ID
  for(int id=0;id<=253;id++)
  {
  dxl_comm_result = packetHandler->write1ByteTxRx(portHandler, id, ADDR_AX_ID, dxl_new_id, &dxl_error);
  delay(10);
  }
  //settorque
  dxl_comm_result = packetHandler->write1ByteTxRx(portHandler, dxl_new_id, ADDR_AX_TORQUE_ENABLE, 1, &dxl_error);
 // delay(100);
  int new_torque_max = 1023;
  int new_torque = 512;
  // or  new_torque 1023, or 512 
  dxl_comm_result = packetHandler->write2ByteTxRx(portHandler, dxl_new_id, ADDR_AX_MAX_TORQUE, new_torque, &dxl_error);
  //delay(10);
  int new_speed = 512;
  dxl_comm_result = packetHandler->write2ByteTxRx(portHandler, dxl_new_id, ADDR_AX_MOVING_SPEED , new_speed, &dxl_error);
  
}


void loop() {
  /* Read my program running time in milliseconds */
  time = millis();

  //NEURON PART
  /* After 5 seconds, inject a current in the first neuron for a duration of 0.01 second*/
  if((time>5000)&&(time<5010))
  rs_neuron[0].inj_cur = 1;
  else
  rs_neuron[0].inj_cur = 0;

  /* Update the neurons output*/
  update_locomotion_network();
  
  packetHandler->read1ByteTxRx(portHandler, dxl_new_id, MOVING, (uint8_t*)&isMoving, &dxl_error);

  if( isMoving == 0 ){ //if Dynamixel is stopped
    // set goal position
    goalPosition = (rs_neuron[0].V *50)+250

    //Send instruction packet to move for goalPosition
    dxl_comm_result = packetHandler->write2ByteTxRx(portHandler, dxl_new_id, ADDR_AX_GOAL_POSITION, goalPosition, &dxl_error);

    packetHandler->read2ByteTxRx(portHandler, dxl_new_id, ADDR_AX_PRESENT_POSITION, (uint16_t*)&dxl_present_position, &dxl_error);
    Serial.print("ID : ");
    Serial.print(dxl_new_id);
    Serial.print("\t Present Position : ");
    Serial.print(dxl_present_position);
    Serial.print("\n");
  }
   
}