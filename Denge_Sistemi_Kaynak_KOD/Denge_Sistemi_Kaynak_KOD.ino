#include<Wire.h>;
 
const int MPU_addr = 0x68;
float kalman_old = 0;
float cov_old = 1;
float val1, val2, val3, val4, val5;
int med1_sort, med2_sort, med3_sort, med4_sort;
 
float avg;
int med;
int m;
int med_sort[5];
int c_avg = 1;
int c_med = 1;
int d = 0;
float old_x = 0;
float real_angle = 0;
float prev_angle = 0;
 
unsigned long previousMillis = 0;
const long interval = 50;

#define sag_motor 3
#define sol_motor 5

int x ;
int xeski;
int y; 
void setup() {
  Wire.begin();
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);
  Wire.write(0x00);
  Wire.endTransmission(true);
  delay(50);
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x1C);
  Wire.write(0x00);
  Wire.endTransmission(true);
  Serial.begin(9600);
  pinMode(3,OUTPUT);
  pinMode(5,OUTPUT);
}
 
void loop() {
  unsigned long currentMillis = millis();
 
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
 
    Wire.beginTransmission(MPU_addr);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU_addr, 2, true);
    int16_t XAxisFull =  (Wire.read() << 8 | Wire.read());
 
    float XAxisFinal = (float) XAxisFull / 16384.0;
 
 
    int XAxis_Filtered = general_filter(c_avg, c_med, XAxisFull);
    int XAxis_kalman = kalman_filter(XAxisFull);
    int XAxis_movavg = mov_avg(c_avg, XAxisFull);
    int XAxis_median = median(c_med, XAxisFull);
 
    Serial.print(XAxis_Filtered);
    Serial.print("\t");
    //Serial.println(XAxis_kalman);
    //Serial.print("\t");
    //Serial.println(XAxis_movavg);
    //Serial.print("\t");
    //Serial.println(XAxis_median);
    //Serial.print("\t");
    //Serial.println(XAxisFull);
 
    c_avg = c_avg + 1;
    c_med = c_med + 1;
 
    if (c_avg > 5 && c_med > 5)
    {
      c_avg = 6;
      c_med = 6;
    }
  
  //Alçak geçiren filtre
  x=XAxis_Filtered ; 
  y=0.1*x+(0.9)*xeski ;  
  xeski=x;

  //Motorlara güç verdiğimiz kısım
  if(y<0){
    analogWrite(sol_motor,(100));
    analogWrite(sag_motor,(100+(-XAxis_Filtered/250)));
  }
  if(y>0){
    analogWrite(sol_motor,(100+(XAxis_Filtered)/250));   
    analogWrite(sag_motor,(100));
  }
  }
  Serial.println(y);
  
}
// Kalmanfiltresi Fonksiyonu
float kalman_filter (float input)
{
 
  float kalman_new = kalman_old;
  float cov_new = cov_old + 0.50;
 
  float kalman_gain = cov_new / (cov_new + 0.9);
  float kalman_calculated = kalman_new + (kalman_gain * (input - kalman_new));
 
  cov_new = (1 - kalman_gain) * cov_old;
  cov_old = cov_new;
 
  kalman_old = kalman_calculated;
 
  return kalman_calculated;
 
}
 //Hareketli Ortalamalar Fonksiyonu
float mov_avg (int counter, float input)
{
  avg = 0;
  m = 0;
 
  if (counter == 1) {
    val1 = input;
    avg = val1;
   }
  else if (counter == 2) {
    val2 = input;
    avg = val2;
  }
  else if (counter == 3) {
    val3 = input;
    avg = val3; 
  }
  else if (counter == 4) {
    val4 = input;
    avg = val4; 
  }
  else if (counter == 5) {
    val5 = input;
    avg = val5;
  }
  else if (counter > 5 ) {
    counter = 6;
    if (val1 == 0) {
      m = m + 1;
    }
    if (val2 == 0) {
      m = m + 1;
    }
    if (val3 == 0) {
      m = m + 1;
    }
    if (val4 == 0) {
      m = m + 1;
    }
    if (val5 == 0) {
      m = m + 1;
    }
    if (input == 0) {
      m = m + 1;
    }
 
    d = 6 - m;
 
 
    if (d == 0)
    {
      avg = input;
      counter = 1;
    }
    else
    {
      avg = (val1 + val2 + val3 + val4 + val5 + input) / d;
    }
 
    val1 = val2;
    val2 = val3;
    val3 = val4;
    val4 = val5;
    val5 = input;
  }
 
  return avg;
}
// Median Filtresi Fonksiyonu
float median (int counter, int input)
{
  if (counter == 1) {
    med1_sort = input;
    med_sort[0] = med1_sort;
 
  }
  else if (counter == 2) {
    med2_sort = input;
    med_sort[1] = med2_sort;
 
  }
  else if (counter == 3) {
    med3_sort = input;
    med_sort[2] = med3_sort;
 
  }
  else if (counter == 4) {
    med4_sort = input;
    med_sort[3] = med4_sort;
 
  }
 
  else if (counter >= 5) {
 
    counter = 6;
 
    med_sort[4] = input;
 
    sort(med_sort, 5);
 
    med = med_sort[2];
 
    med1_sort = med2_sort;
    med2_sort = med3_sort;
    med3_sort = med4_sort;
    med4_sort = input;
 
    med_sort[0] = med1_sort;
    med_sort[1] = med2_sort;
    med_sort[2] = med3_sort;
    med_sort[3] = med4_sort;
 
  }
  return med;
}
// Veri Kaydırma Fonksiyonu
void sort(int a[], int size) {
  for (int i = 0; i < (size - 1); i++) {
    for (int o = 0; o < (size - (i + 1)); o++) {
      if (a[o] > a[o + 1]) {
        int t = a[o];
        a[o] = a[o + 1];
        a[o + 1] = t;
      }
    }
  }
}
 
//----------------------------------------------- 
//Bütün Filtrelerin Uygulandığı Fonksiyon
float general_filter (int counter_avg, int counter_med, float input) {
 

  float input_movavg = mov_avg(counter_avg,input ) ;    //Moving Avg Uygulaması
  float input_med = median(counter_med, input_movavg); //Median Uygulaması
  float input_filtered =kalman_filter(input_med);     //Kalman Uygulaması
  
  return input_filtered;
}
