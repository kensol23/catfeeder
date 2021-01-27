// @author Kener Solorzano Farrier

#include <Servo.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <AnalogMultiButton.h>
#include <TimeLib.h>
#include <TimeAlarms.h>
#include <RTClib.h>
#include <AT24CX.h>

#define PIN_SERVO 9
#define PIN_TECLADO A0
#define PIN_BUZZER A2
#define PIN_AUX A3
#define POSH1 0
#define POSM1 1
#define POSS1 2
#define POSH2 3
#define POSM2 4
#define POSS2 5
#define POSH3 6
#define POSM3 7
#define POSS3 8
#define POSH4 9
#define POSM4 10
#define POSS4 11
#define POSH5 12
#define POSM5 13
#define POSS5 14
#define POSDISPENSADOS 15
#define POSRACION 16
#define POSPORCIONES 17

const int BOTONES_NUMERO = 4;
const int BOTONES_VALORES[BOTONES_NUMERO] = {0, 343, 526, 638};


// ES: Orden de los botones de acuerdo a los valores listados en el arreglo BOTONES_VALORES
// EN: Button order according to values order in BOTONES_VALORES array
const int BOTON_OK      = 0;
const int BOTON_MENU    = 1;
const int BOTON_ARRIBA  = 2;
const int BOTON_ABAJO   = 3;

const int PAG_PRINCIPAL = 0;
const int PAG_RELOJ = 1;
const int PAG_COMIDA = 2;
const int PAG_CANTIDAD = 3;
const int PAG_PORCIONES = 4;

const int RELOJ_A = 0;
const int RELOJ_M = 1;
const int RELOJ_D = 2;
const int RELOJ_h = 3;
const int RELOJ_m = 4;
const int RELOJ_s = 5;

unsigned int tiempo[6]  = { 2021, 1, 1, 0, 0, 0 };  // 0 aC1o 1 mes 2 dia  3 hora 4 munito 5 segundo
unsigned int alarma1[3] = { 0, 0, 0 };  // hora minut segundo alarma 1
unsigned int alarma2[3] = { 0, 0, 0 };  // hora minut segundo alarma 2
unsigned int alarma3[3] = { 0, 0, 0 };  // hora minut segundo alarma 3
unsigned int alarma4[3] = { 0, 0, 0 };  // hora minut segundo alarma 4
unsigned int alarma5[3] = { 0, 0, 0 };  // hora minut segundo alarma 5

AlarmID_t Alarm1id;
AlarmID_t Alarm2id;
AlarmID_t Alarm3id;
AlarmID_t Alarm4id;
AlarmID_t Alarm5id;
AlarmID_t Alarm6id;

const char DIAS[7][12] = { "DOM", "LUN", "MAR", "MIE", "JUE", "VIE", "SAB" };

// La medida de la racion es equivalente a la cantidad de milisegundos
// durante los cuales es necesario dejar la tapa de la tolva abierta para 
// dispensar aproximadamente 1/4 de taza de alimento
unsigned int racion = 300;

unsigned int porciones = 2;

int dispensadosXdia = 3;

int pagina = 0;
int paginas = 5;
int paginasAnterior = 5;
int paginaAnterior = 0;
bool editando, editandoItem = false;

AnalogMultiButton teclado(PIN_TECLADO, BOTONES_NUMERO, BOTONES_VALORES);
LiquidCrystal_I2C lcd (0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display
RTC_DS1307 rtc;
AT24C32 EepromRTC;
Servo miservo;

int segundo,minuto,hora,dia,dd,mes;
long anio; //variable año
DateTime HoraFecha;

void imprError (String e)
{
    lcd.setCursor(0,0);
    lcd.print(e);
    Serial.print(e);
    while (1);
}

void establecerHora ()
{
  rtc.begin(); //Inicializamos el RTC
  Serial.println("Estableciendo Hora y fecha...");
  rtc.adjust(DateTime(tiempo[0],tiempo[1],tiempo[2],tiempo[3],tiempo[4],tiempo[5]));
  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  if (!rtc.isrunning())
  {
    lcd.setCursor (0, 0);
    lcd.print ("Reloj no esta en hora!");
    Serial.println ("Reloj no esta en hora!");
  } else {
    lcd.clear();
    lcd.setCursor (0, 0);
    lcd.print ("Reloj Configurado!");
    Serial.println ("Reloj Configurado!");
  }
}

void imprimeFecha ()
{
  HoraFecha = rtc.now();
  
  segundo = HoraFecha.second();
  minuto  = HoraFecha.minute();
  hora    = HoraFecha.hour();
  dia     = HoraFecha.dayOfTheWeek();
  dd      = HoraFecha.day();
  mes     = HoraFecha.month();
  anio    = HoraFecha.year();

  lcd.clear();
  lcd.print(DIAS[dia]);
  lcd.print(" ");
  lcd.print(dd);
  lcd.print(" ");
  lcd.print(hora);
  lcd.print(":");
  lcd.print(minuto);
  lcd.print(":");
  lcd.print(segundo);
}

void probarEEPROM ()
{
  byte variable_byte=235;
  int variable_int=3000;
  long variable_long=48300011;
  float variable_float=3.14;
  char cadena[30] = "Naylamp Mechatronics";
  
  Serial.println("Guardando datos en la EEPROM...");
  
  EepromRTC.write(1,variable_byte); // posiscion 1:ocupa 1 byte de tamaño
  EepromRTC.writeInt(2,variable_int); // posiscion 2:ocupa 2 bytes de tamaño 
  EepromRTC.writeLong(4,variable_long); // posiscion 4: ocupa 4 bytes de tamaño 
  EepromRTC.writeFloat(8,variable_float); // posiscion 8:ocupa 4 bytes de tamaño 
  EepromRTC.writeChars(12, cadena, 30);// posiscion 16 y  20 bytes de tamaño 

  
  Serial.println("Leyendo datos guardados...");

  byte a = EepromRTC.read(1);
  int b = EepromRTC.readInt(2);
  long c = EepromRTC.readLong(4);
  float d = EepromRTC.readFloat(8);
  char cadena2[30];
  EepromRTC.readChars(12,cadena2,30);

  Serial.print("Dato byte: ");Serial.println(a);
  Serial.print("Dato int: "); Serial.println(b);
  Serial.print("Dato long: "); Serial.println(c);
  Serial.print("Dato float: "); Serial.println(d);
  Serial.print("Dato Cadena : "); Serial.println(cadena2);
  Serial.println();

  delay(5000);
}

void crearComidas ()
{
  int h,m,s = 0;

  EepromRTC.writeInt(POSH1,h);
  EepromRTC.writeInt(POSM1,m);
  EepromRTC.writeInt(POSS1,s);
  
  EepromRTC.writeInt(POSH2,h);
  EepromRTC.writeInt(POSM2,m);
  EepromRTC.writeInt(POSS2,s);

  EepromRTC.writeInt(POSH3,h);
  EepromRTC.writeInt(POSM3,m);
  EepromRTC.writeInt(POSS3,s);

  EepromRTC.writeInt(POSH4,h);
  EepromRTC.writeInt(POSM4,m);
  EepromRTC.writeInt(POSS4,s);

  EepromRTC.writeInt(POSH5,h);
  EepromRTC.writeInt(POSM5,m);
  EepromRTC.writeInt(POSS5,s);

  EepromRTC.writeInt(POSDISPENSADOS,dispensadosXdia);
  EepromRTC.writeInt(POSRACION,racion);
}

void dispensar ()
{
  digitalWrite (PIN_AUX, HIGH);

  miservo.write(180);
  delay(racion*porciones);
  miservo.write(0);
  
  digitalWrite (PIN_AUX, LOW);
}

void imprAlarm (unsigned int a[3])
{
  Serial.print (a[0], DEC);
  Serial.print (":");
  Serial.print (a[1], DEC);
  Serial.print (":");
  Serial.println (a[2], DEC);
}

void activaAlarma (AlarmID_t alarmaId, unsigned int alarma[3])
{
  if (alarma[0] != 0 || alarma[1] != 0)
  {
    alarmaId = Alarm.alarmRepeat (alarma[0], alarma[1], alarma[2], dispensar);
  }
}

void iniciaComidas ()
{
  alarma1[0] = EepromRTC.read(POSH1);
  alarma1[1] = EepromRTC.read(POSM1);
  alarma1[2] = EepromRTC.read(POSS1);
  imprAlarm(alarma1);

  alarma2[0] = EepromRTC.read(POSH2);
  alarma2[1] = EepromRTC.read(POSM2);
  alarma2[2] = EepromRTC.read(POSS2);
  imprAlarm(alarma2);

  alarma3[0] = EepromRTC.read(POSH3);
  alarma3[1] = EepromRTC.read(POSM3);
  alarma3[2] = EepromRTC.read(POSS3);
  imprAlarm(alarma3);

  alarma4[0] = EepromRTC.read(POSH4);
  alarma4[1] = EepromRTC.read(POSM4);
  alarma4[2] = EepromRTC.read(POSS4);
  imprAlarm(alarma4);

  alarma5[0] = EepromRTC.read(POSH5);
  alarma5[1] = EepromRTC.read(POSM5);
  alarma5[2] = EepromRTC.read(POSS5);
  imprAlarm(alarma4);

  dispensadosXdia = EepromRTC.read(POSDISPENSADOS);
  racion          = EepromRTC.read(POSRACION);
  porciones       = EepromRTC.read(POSPORCIONES);

  activaAlarma (Alarm1id, alarma1);
  activaAlarma (Alarm2id, alarma2);
  activaAlarma (Alarm3id, alarma3);
  activaAlarma (Alarm4id, alarma4);
  activaAlarma (Alarm5id, alarma5);

}

void muestraPagina(){
  switch (pagina)
  {
    case PAG_PRINCIPAL:
      paginaPrincipal();
    break;

    case PAG_RELOJ:
      paginaReloj();
    break;
    
    case PAG_COMIDA:
      paginaTiemposComidas();
    break;

    case PAG_CANTIDAD:
      paginaCantidad();
    break;

    case PAG_PORCIONES:
      paginaPorciones();
    break;

    default:
    break;
  }
}

void paginaPrincipal () {
  lcd.clear();
  lcd.setCursor(0,0);
  imprimeFecha();
}

void paginaReloj () {
  
  lcd.clear ();
  lcd.setCursor (0, 0);
  lcd.print ("Ajustar Reloj:");
  if (editando)
  {
    bool navegando = true;

    paginaAnterior = pagina;
    pagina = 0;

    paginasAnterior = paginas;
    paginas = 6;

    tiempo[0] = anio;
    tiempo[1] = mes;
    tiempo[2] = dd;
    tiempo[3] = hora;
    tiempo[4] = minuto;
    tiempo[5] = segundo;

    while(navegando){
      pantallaEdicion();
      teclado.update();

      if (teclado.onPress(BOTON_MENU))
      {
        navegando = false;
        pagina = paginaAnterior;
        paginas = paginasAnterior;
      }
      if (teclado.onPress(BOTON_ARRIBA))
      {
        subir();
      }
      if (teclado.onPress(BOTON_ABAJO))
      {
        bajar();
      }
      if (teclado.onPress(BOTON_OK))
      {
        bool editando_reloj = true;
        lcd.setCursor(6,1);
        lcd.print("*");

        while (editando_reloj){
          teclado.update();
          
          if (teclado.onPress(BOTON_ARRIBA))
          {
            tiempo[pagina]++;
            
            lcd.setCursor(7,1);
            lcd.print("    ");
            lcd.setCursor(7,1);
            lcd.print(tiempo[pagina]);
          }
          if (teclado.onPress(BOTON_ABAJO))
          {
            tiempo[pagina]--;

            lcd.setCursor(7,1);
            lcd.print("    ");
            lcd.setCursor(7,1);
            lcd.print(tiempo[pagina]);
          }
          if (teclado.onPress(BOTON_MENU))
          {
            editando_reloj = false;
          }

          delay(200);
        }
      }

        switch (pagina)
        {
          case RELOJ_A:
            lcd.setCursor(2,1);
            lcd.print("Ano: ");
            lcd.setCursor(7,1);
            lcd.print("    ");
            lcd.setCursor(7,1);
            lcd.print(tiempo[pagina]);
          break;

          case RELOJ_M:
            lcd.setCursor(2,1);
            lcd.print("Mes: ");
            lcd.setCursor(7,1);
            lcd.print("    ");
            lcd.setCursor(7,1);
            lcd.print(tiempo[pagina]);
          break;
          
          case RELOJ_D:
            lcd.setCursor(2,1);
            lcd.print("Dia: ");
            lcd.setCursor(7,1);
            lcd.print("    ");
            lcd.setCursor(7,1);
            lcd.print(tiempo[pagina]);
          break;

          case RELOJ_h:
            lcd.setCursor(2,1);
            lcd.print("Hor: ");
            lcd.setCursor(7,1);
            lcd.print("    ");
            lcd.setCursor(7,1);
            lcd.print(tiempo[pagina]);
          break;

          case RELOJ_m:
            lcd.setCursor(2,1);
            lcd.print("Min: ");
            lcd.setCursor(7,1);
            lcd.print("   ");
            lcd.setCursor(7,1);
            lcd.print(tiempo[pagina]);
          break;

          case RELOJ_s:
            lcd.setCursor(2,1);
            lcd.print("Seg: ");
            lcd.setCursor(7,1);
            lcd.print("    ");
            lcd.setCursor(7,1);
            lcd.print(tiempo[pagina]);
          break;

          default:
          break;
        }
      delay(200);
    }
  }
}

void paginaTiemposComidas () {
  lcd.clear ();
  lcd.setCursor (0, 0);
  lcd.print ("Comidas...");
  if (editando)
  {
    pantallaEdicion();
  }
}

void paginaCantidad () {
  lcd.clear ();
  lcd.setCursor (0, 0);
  lcd.print ("Cantidad...");
  if (editando)
  {
    pantallaEdicion();
  }
}

void paginaPorciones () {
  lcd.clear ();
  lcd.setCursor (0, 0);
  lcd.print ("Porciones...");
  if (editando)
  {
    pantallaEdicion();
  }
}

void subir (){
  pagina++;
  if(pagina >= paginas) 
  {
    pagina = 0;
  }
}

void bajar (){
  pagina--;
  if(pagina == -1) 
  {
    pagina = paginas;
  } 
}

void pantallaEdicion ()
{
  lcd.setCursor(0,1);
  lcd.print("> ");
}
//
void setup() {
  Serial.begin(9600);
  Wire.begin();
  lcd.init();
  lcd.backlight();

  //probarEEPROM();

  miservo.attach (PIN_SERVO);   // Initialize Servo
  pinMode (PIN_BUZZER, OUTPUT); // Initialize buzzer
  pinMode (PIN_AUX, OUTPUT);
  miservo.write (0);            // Set servo position to 0 degrees.

  rtc.begin();

  DateTime now = rtc.now();
  setTime (now.hour (), now.minute (), now.second (), now.month (), now.day (), now.year ()); // set time to Saturday 8:29:00am Jan 1 2011

  // Print a message to the LCD.
  
  lcd.setCursor(0,0);
  lcd.print("Hola, mundo!");
  lcd.setCursor(0,1);
  lcd.print("Ken Arduino!");
}

void loop() {
  
  DateTime now = rtc.now();
  teclado.update();

  if (teclado.onPress(BOTON_ARRIBA))
  {
    subir();
  }
  if (teclado.onPress(BOTON_ABAJO))
  {
    bajar();
  }
  if (teclado.onPress(BOTON_OK))
  {
    if (pagina != 0){
      editando = true;
      while (editando){
        teclado.update();

        muestraPagina();

        if (teclado.onPress(BOTON_MENU))
        {
          editando = false;
        }
        delay(200);
      }
    } else { // 

    }
  }
  if (teclado.onPress(BOTON_MENU))
  {
    Serial.println("MENU!!");
  }

  muestraPagina();

  delay(200);
}
