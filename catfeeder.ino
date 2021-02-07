// @author Kener Solorzano Farrier

#include <Servo.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <AnalogMultiButton.h>
#include <TimeLib.h>
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
#define POSDISPENSADO1 18
#define POSDISPENSADO2 19
#define POSDISPENSADO3 20
#define POSDISPENSADO4 21
#define POSDISPENSADO5 22
#define POSFECHA 23

#define GLOBALDELAY 100
#define DELAYAFTERPROMPT 2000

const int BOTONES_NUMERO = 4; // cantidad de botones en el teclado
const int BOTONES_VALORES[BOTONES_NUMERO] = {0, 343, 526, 638}; // voltajes correspondietes a cada uno de los botones en su respectivo orden

// Orden de los botones de acuerdo a los valores listados en el arreglo BOTONES_VALORES
const int OK      = 0;
const int MENU    = 1;
const int ARRIBA  = 2;
const int ABAJO   = 3;

// Indices Menu Principal
const int PAG_PRINCIPAL = 0;
const int PAG_RELOJ = 1;
const int PAG_COMIDA = 2;
const int PAG_CANTIDAD = 3;
const int PAG_PORCIONES = 4;

// Indices Menu Reloj
const int RELOJ_A = 0;
const int RELOJ_M = 1;
const int RELOJ_D = 2;
const int RELOJ_h = 3;
const int RELOJ_m = 4;
const int RELOJ_s = 5;

// Indices Menu Alarmas
const int ALARMA_1 = 0;
const int ALARMA_2 = 1;
const int ALARMA_3 = 2;
const int ALARMA_4 = 3;
const int ALARMA_5 = 4;

const int HORA = 0;
const int MINUTO = 1;
const int SEGUNDO = 2;

const char DIAS[7][12] = { "DOM", "LUN", "MAR", "MIE", "JUE", "VIE", "SAB" }; // Dias de la semana desplejados por el reloj de la pagina principal
const int POS_COMIDAS_DISPENSADAS[5] = {POSDISPENSADO1, POSDISPENSADO2, POSDISPENSADO3, POSDISPENSADO4, POSDISPENSADO5};

const uint8_t pendiente[8]  = {0xe, 0x15, 0x15, 0x13, 0xe, 0x0, 0x11, 0x1f};  // Icono comida pendiente
const uint8_t dispensada[8] = {0x1, 0xa, 0x4, 0x0, 0x4, 0xa, 0x15, 0x1f};     // Icono comdia dispensada
const uint8_t saltada[8]    = {0x11, 0xa, 0x4, 0xa, 0x11, 0x0, 0x11, 0x1f};   // Icono comida saltada
const uint8_t manual[8]     = {0x4, 0x15, 0xe, 0x4, 0x0, 0xa, 0x15, 0x1f};    // Icono comida manual

int racion           = 300; // La medida de la racion es equivalente a la cantidad de milisegundos durante los cuales es necesario dejar la tapa de la tolva abierta para durante los cuales es necesario dejar la tapa de la tolva abierta para
int porciones        = 2;   // cantidad de porciones por servida
int dispensadosXdia  = 3;   // cantidad de servidas dispensadas por dia

int pagina  = 0;
int paginas = 5;
int paginasAnterior = 5;
int paginaAnterior  = 0;
bool editando, editandoItem = false;

int tiempo[6]               = { 2021, 1, 1, 0, 0, 0 };  // 0 anho 1 mes 2 dia  3 hora 4 minuto 5 segundo
int alarmas[5][3]           = {{ 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }};
int comidasServidas[5]      = {0, 0, 0, 0, 0};
int ultimoDiaDispensado     = 0;
int comidasSaltadas         = 0;
String pantallasAlarmas[5]  = {"Comida 1", "Comida 2", "Comida 3", "Comida 4", "Comida 5"};
boolean imprimirGrafico = true;

AnalogMultiButton teclado(PIN_TECLADO, BOTONES_NUMERO, BOTONES_VALORES);
LiquidCrystal_I2C lcd (0x27,16,2);  // Configura la direccion del lcd a 0x27 con una pantalla de 16x2
RTC_DS1307        rtc;
AT24C32           EepromRTC;
Servo             miservo;

String h = "";
String m = "";
String s = "";

int mesesLargos[7] = {1, 3, 5, 7, 8, 10, 12};
int mesesCortos[4] = {4, 6, 9, 11};
int segundo, minuto, hora, dia, dd, mes;
long anio; 
DateTime HoraFecha;

String timeDigit (int d)
{
  if(String(d).length() > 1)
  {
    return String(d);
  }
  return "0" + String(d);
}

void beep ()
{

  for (int i = 0; i < 100; i++)
    {
      digitalWrite (PIN_BUZZER, LOW);
      delayMicroseconds (200);
      digitalWrite (PIN_BUZZER, HIGH);
      delayMicroseconds (200);
    }

}

bool meslargo ()
{
  bool found = false;
  for (int i = 0; i < 7; i++)
  {
    if (mesesLargos[i]==tiempo[RELOJ_M])
    {
      found = true;
    }
  }
  return found;
}

bool mescorto ()
{
  bool found = false;
  for (int i = 0; i < 4; i++)
  {
    if (mesesCortos[i]==tiempo[RELOJ_M])
    {
      found = true;
    }
  }
  return found;
}

void imprError (String e)
{
    lcd.setCursor(0,0);
    lcd.print(e);
    Serial.print(e);
    while (1);
}

void establecerHora ()
{
  rtc.begin(); //Inicializar el RTC
  Serial.println("Estableciendo Hora y fecha...");
  rtc.adjust(DateTime(tiempo[0],tiempo[1],tiempo[2],tiempo[3],tiempo[4],tiempo[5]));
  if (!rtc.isrunning())
  {
    Serial.println ("Reloj no esta en hora!");
  } else {
    Serial.println ("Actualizado!");
  }
}

void imprimeFecha ()
{
  lcd.setCursor(0,0);
  lcd.print(DIAS[dia]);
  lcd.print("-");
  lcd.print(timeDigit(dd));
  lcd.print("  ");
  lcd.print(timeDigit(hora));
  lcd.print(":");
  lcd.print(timeDigit(minuto));
  lcd.print(":");
  lcd.print(timeDigit(segundo));
}

void imprimeGraficoComidasAlt ()
{
  lcd.setCursor(0,1);
  lcd.print("                ");
  for(int x=0; x<16; x++)
  {
    lcd.setCursor(x,1);
    for(int i=0; i<5; i++)
    {
      if(alarmas[i][0] != 0 || alarmas[i][1] != 0)
      {
        int pos = (int)((alarmas[i][0] * 16) / 24);
        if(x == pos)
        {
          lcd.write(byte(comidasServidas[i]));
        } 
      } 
    }
  }
}

void imprimeGraficoComidas ()
{
  calculaComidasSaltadas();

  lcd.setCursor(0,1);
  lcd.print("        ");
  lcd.setCursor(0,1);
  lcd.print("[");
  for(int i=0; i<5; i++)
  {
    lcd.setCursor(i+1,1);
    if(alarmas[i][0] != 0 || alarmas[i][1] != 0)
    {
      if(comidasServidas[i] != 0)
      {
        lcd.write(byte(1));
      } else {
        if((alarmas[i][0] == hora && alarmas[i][1] > minuto) || alarmas[i][0] > hora)
        {
          lcd.write(byte(0));
        } else {
          lcd.write(byte(2));
        }
      }
    } else {
      lcd.print("_");
    }
  }
  lcd.print("]");
}

void guardaAlarmas ()
{
  EepromRTC.writeInt(POSH1, alarmas[0][0]);
  EepromRTC.writeInt(POSM1, alarmas[0][1]);
  EepromRTC.writeInt(POSS1, alarmas[0][2]);

  EepromRTC.writeInt(POSH2, alarmas[1][0]);
  EepromRTC.writeInt(POSM2, alarmas[1][1]);
  EepromRTC.writeInt(POSS2, alarmas[1][2]);

  EepromRTC.writeInt(POSH3, alarmas[2][0]);
  EepromRTC.writeInt(POSM3, alarmas[2][1]);
  EepromRTC.writeInt(POSS3, alarmas[2][2]);

  EepromRTC.writeInt(POSH4, alarmas[3][0]);
  EepromRTC.writeInt(POSM4, alarmas[3][1]);
  EepromRTC.writeInt(POSS4, alarmas[3][2]);

  EepromRTC.writeInt(POSH5, alarmas[4][0]);
  EepromRTC.writeInt(POSM5, alarmas[4][1]);
  EepromRTC.writeInt(POSS5, alarmas[4][2]);

  EepromRTC.writeInt(POSDISPENSADOS,dispensadosXdia);
  EepromRTC.writeInt(POSRACION,racion);

  Serial.println("Datos Actualizados!");
}

void dispensar ()
{
  beep();
  beep();
  beep();
  digitalWrite (PIN_AUX, HIGH);

  miservo.write(180);
  delay(racion*porciones);
  miservo.write(0);

  digitalWrite (PIN_AUX, LOW);
  beep();
  beep();
  beep();

  for(int i = 0; i<5; i++)
  {
    if(alarmas[i][0] == hora && alarmas[i][1] == minuto && alarmas[i][2] == segundo)
    {
      comidasServidas[i] = 1;
      EepromRTC.writeInt(POS_COMIDAS_DISPENSADAS[i],1);
      EepromRTC.writeInt(POSFECHA, HoraFecha.day());
    }
  }
  if(pagina == 0){
    imprimeGraficoComidas();
  }
}

void restablecerComidasDispensadas ()
{
  //Serial.print("BINGO!!!");
  for(int i = 0; i<5; i++)
  {
    EepromRTC.writeInt(POS_COMIDAS_DISPENSADAS[i], 0);
  }
  EepromRTC.writeInt(POSFECHA, HoraFecha.day());
}

void cargarComidasDispensadas ()
{
  for(int i=0; i<5; i++)
  {
    comidasServidas[i] = EepromRTC.read(POS_COMIDAS_DISPENSADAS[i]);
  }
  ultimoDiaDispensado = EepromRTC.read(POSFECHA);
}

void imprAlarm (int a[3])
{
  Serial.print (a[0], DEC);
  Serial.print (":");
  Serial.print (a[1], DEC);
  Serial.print (":");
  Serial.println (a[2], DEC);
}

void cargaDatos ()
{
  alarmas[0][0] = EepromRTC.read(POSH1);
  alarmas[0][1] = EepromRTC.read(POSM1);
  alarmas[0][2] = EepromRTC.read(POSS1);
  imprAlarm(alarmas[0]);

  alarmas[1][0] = EepromRTC.read(POSH2);
  alarmas[1][1] = EepromRTC.read(POSM2);
  alarmas[1][2] = EepromRTC.read(POSS2);
  imprAlarm(alarmas[1]);

  alarmas[2][0] = EepromRTC.read(POSH3);
  alarmas[2][1] = EepromRTC.read(POSM3);
  alarmas[2][2] = EepromRTC.read(POSS3);
  imprAlarm(alarmas[2]);

  alarmas[3][0] = EepromRTC.read(POSH4);
  alarmas[3][1] = EepromRTC.read(POSM4);
  alarmas[3][2] = EepromRTC.read(POSS4);
  imprAlarm(alarmas[3]);

  alarmas[4][0] = EepromRTC.read(POSH5);
  alarmas[4][1] = EepromRTC.read(POSM5);
  alarmas[4][2] = EepromRTC.read(POSS5);
  imprAlarm(alarmas[4]);

  dispensadosXdia = EepromRTC.read(POSDISPENSADOS);
  racion          = EepromRTC.read(POSRACION);
  porciones       = EepromRTC.read(POSPORCIONES);

  cargarComidasDispensadas();

  Serial.println("Datos Cargados!");
  delay(DELAYAFTERPROMPT);
}

void iniciaComidas ()
{
  cargaDatos ();
  Serial.println("Comidas Iniciadas!");
}

void muestraPagina (){
  switch (pagina)
  {
    case PAG_PRINCIPAL:
      imprimePaginaPrincipal();
    break;

    case PAG_RELOJ:
      imprimePaginaReloj();
    break;

    case PAG_COMIDA:
      imprimePaginaTiemposComidas();
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

void imprimePaginaPrincipal () {
  //lcd.clear();
  imprimeFecha();
  if(imprimirGrafico)
  {
    imprimeGraficoComidas();
    imprimirGrafico = false;
  }
}

void imprimePaginaReloj ()
{
  lcd.clear ();
  lcd.setCursor (0, 0);
  lcd.print ("Reloj:");

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

      if (teclado.onPress(MENU)) // Al presionar MENU en modo de navegacion
      {
        navegando = false;
        pagina = paginaAnterior;
        paginas = paginasAnterior;
        establecerHora ();
      }
      if (teclado.onPress(ARRIBA))
      {
        subir();
      }
      if (teclado.onPress(ABAJO))
      {
        bajar();
      }
      if (teclado.onPress(OK))
      {
        bool editando_reloj = true;
        lcd.setCursor(6,1);
        lcd.print("*");

        while (editando_reloj){
          teclado.update();

          if (teclado.onPress(ARRIBA)) // Al presionar el boton arriba.
          {
            tiempo[pagina]++; // Sumamos uno al elemento de la fecha que esta siendo editado.

            switch (pagina)   // Comprobamos que parte de la fecha es
            {
              case RELOJ_A: // Si es el Anho no hacer nada; no hay limite para el anho.
              break;

              case RELOJ_M: // Si es el Mes se comprueba que no sea mayor a 12 si es asi, retornamos a 1
                if (tiempo[RELOJ_M] > 12){
                  tiempo[RELOJ_M] = 1;
                }
              break;

              case RELOJ_D: // Si es el Dia
                if (tiempo[RELOJ_M] == 2) // Si es el mes de febrero
                {
                  if ( (tiempo[RELOJ_A]%4==0) && ( (tiempo[RELOJ_A]%400==0) || (tiempo[RELOJ_A]%100!=0) ) ) // Si es anho bisiesto
                  {
                    if(tiempo[RELOJ_D] > 29) // Al sobrepasar el dia 29
                    {
                      tiempo[RELOJ_D] = 1; // Volver al dia 1
                    }
                  }
                  else // Si no es anho bisiesto
                  {
                    if(tiempo[RELOJ_D] > 28) // Al sobrepasar el dia 28
                    {
                      tiempo[RELOJ_D] = 1; // Volver al dia 1
                    }
                  }
                }
                else if (meslargo()) // Comprueba si es un mes de 31 dias.
                {
                  if(tiempo[RELOJ_D] > 31) // Al sobrepasar el dia 31
                  {
                    tiempo[RELOJ_D] = 1; // Volver al dia 1
                  }
                }
                else if (mescorto()) // Comprueba si es un mes de 30 dias.
                {
                  if(tiempo[RELOJ_D] > 30) // Al sobrepasar el dia 31
                  {
                    tiempo[RELOJ_D] = 1; // Volver al dia 1
                  }
                }
              break;

              case RELOJ_h:
                if(tiempo[RELOJ_h] > 23) // Al sobrepasar la hora 23
                {
                  tiempo[RELOJ_h] = 0; // Volver a la hora 0
                }
              break;

              case RELOJ_m:
                if(tiempo[RELOJ_m] > 59) // Al sobrepasar el minuto 59
                {
                  tiempo[RELOJ_m] = 0; // Volver al minuto 0
                }
              break;

              case RELOJ_s:
                if(tiempo[RELOJ_s] > 59) // Al sobrepasar el segundo 59
                {
                  tiempo[RELOJ_s] = 0; // Volver al segundo 0
                }
              break;

              default:
              break;
            }
            lcd.setCursor(7,1);
            lcd.print("    ");
            lcd.setCursor(7,1);
            lcd.print(tiempo[pagina]);
          }
          if (teclado.onPress(ABAJO))
          {
            tiempo[pagina]--;

            switch (pagina)   // Comprobamos que parte de la fecha es
            {
              case RELOJ_A: // Si es el Anho y llega a Cero re-iniciar en 4000.
                if(tiempo[RELOJ_A] < 0)
                {
                  tiempo[RELOJ_A] = 4000;
                }
              break;

              case RELOJ_M: // Si es el Mes se comprueba que no sea mayor a 12 si es asi, retornamos a 1
                if (tiempo[RELOJ_M] < 1){
                  tiempo[RELOJ_M] = 12;
                }
              break;

              case RELOJ_D: // Si es el Dia
                if (tiempo[RELOJ_M] == 2) // Si es el mes de febrero
                {
                  if ( (tiempo[RELOJ_A]%4==0) && ( (tiempo[RELOJ_A]%400==0) || (tiempo[RELOJ_A]%100!=0) ) ) // Si es anho bisiesto
                  {
                    if(tiempo[RELOJ_D] < 1) // Al bajar del dia 1
                    {
                      tiempo[RELOJ_D] = 29; // Volver al dia 29
                    }
                  }
                  else // Si no es anho bisiesto
                  {
                    if(tiempo[RELOJ_D] < 1) // Al bajar del dia 1
                    {
                      tiempo[RELOJ_D] = 28; // Volver al dia 28
                    }
                  }
                }
                else if (meslargo()) // Comprueba si es un mes de 31 dias.
                {
                  if(tiempo[RELOJ_D] < 1) // Al bajar del dia 1
                  {
                    tiempo[RELOJ_D] = 31; // Volver al dia 31
                  }
                }
                else if (mescorto()) // Comprueba si es un mes de 30 dias.
                {
                  if(tiempo[RELOJ_D] < 1) // Al bajar del dia 1
                  {
                    tiempo[RELOJ_D] = 30; // Volver al dia 30
                  }
                }
              break;

              case RELOJ_h:
                if(tiempo[RELOJ_h] < 0)
                {
                  tiempo[RELOJ_h] = 23;
                }
              break;

              case RELOJ_m:
                if(tiempo[RELOJ_m] < 0)
                {
                  tiempo[RELOJ_m] = 59;
                }
              break;

              case RELOJ_s:
                if(tiempo[RELOJ_s] < 0)
                {
                  tiempo[RELOJ_s] = 59;
                }
              break;

              default:
              break;
            }
            lcd.setCursor(7,1);
            lcd.print("    ");
            lcd.setCursor(7,1);
            lcd.print(tiempo[pagina]);
          }
          if (teclado.onPress(MENU))
          {
            editando_reloj = false;
          }

          delay(GLOBALDELAY);
        }
      }

      if (navegando)
      {
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
            lcd.print(timeDigit(tiempo[pagina]));
          break;

          case RELOJ_m:
            lcd.setCursor(2,1);
            lcd.print("Min: ");
            lcd.setCursor(7,1);
            lcd.print("   ");
            lcd.setCursor(7,1);
            lcd.print(timeDigit(tiempo[pagina]));
          break;

          case RELOJ_s:
            lcd.setCursor(2,1);
            lcd.print("Seg: ");
            lcd.setCursor(7,1);
            lcd.print("    ");
            lcd.setCursor(7,1);
            lcd.print(timeDigit(tiempo[pagina]));
          break;
        }
      }
      delay(GLOBALDELAY);
    }
  }
}

void imprimePaginaTiemposComidas () {
  lcd.clear ();
  lcd.setCursor (0, 0);
  lcd.print ("Comidas:");
  if (editando)
  {
    bool navegando = true;
    paginaAnterior = pagina;
    pagina = 0;
    paginasAnterior = paginas;
    paginas = 5;

    while (navegando)
    {
      teclado.update();
      // Evaluar el input de los botones
      if (teclado.onPress(MENU)) // Al presionar MENU en modo de navegacion
      {
        navegando = false;
        pagina = paginaAnterior;
        paginas = paginasAnterior;
        iniciaComidas ();
      }
      if (teclado.onPress(ARRIBA))
      {
        subir();
      }
      if (teclado.onPress(ABAJO))
      {
        bajar();
      }
      if (teclado.onPress(OK))
      {
        bool editando_alarma = true;
        const int ELEMENTOS = 3;
        int elemento = 0;

        while (editando_alarma)
        {
          teclado.update();

          if (teclado.onPress(MENU)) // Al presionar MENU en modo de navegacion
          {
            editando_alarma = false;
            guardaAlarmas();
          }
          if (teclado.onPress(ARRIBA))
          {
            elemento++;
            if (elemento > ELEMENTOS)
            {
              elemento = 0;
            }
          }
          if (teclado.onPress(ABAJO))
          {
            elemento--;
            if (elemento < 0)
            {
              elemento = ELEMENTOS;
            }
          }
          if (teclado.onPress(OK))
          {
            bool editando_elemento_alarma = true;
            imprimeAlarma(pagina,true);
            while (editando_elemento_alarma)
            {
              teclado.update();
              if (teclado.onPress(MENU))
              {
                editando_elemento_alarma = false;
              }
              if (teclado.onPress(OK))
              {
                editando_elemento_alarma = false;
              }
              if (teclado.onPress(ARRIBA))
              {
                alarmas[pagina][elemento]++;
                switch (elemento)
                {
                  case HORA:
                    if (alarmas[pagina][elemento] > 23)
                    {
                      alarmas[pagina][elemento] = 0;
                    }
                    break;
                  default:
                    if (alarmas[pagina][elemento] > 59)
                    {
                      alarmas[pagina][elemento] = 0;
                    }
                    break;
                }
              }
              if (teclado.onPress(ABAJO))
              {
                alarmas[pagina][elemento]--;
                switch (elemento)
                {
                  case HORA:
                  Serial.println(alarmas[pagina][elemento]);
                    if (alarmas[pagina][elemento] < 0)
                    {
                      alarmas[pagina][elemento] = 23;
                    }
                    break;
                  default:
                    if (alarmas[pagina][elemento] < 0)
                    {
                      alarmas[pagina][elemento] = 59;
                    }
                    break;
                }
              }
              imprimeAlarma(pagina,true);
              delay(GLOBALDELAY);
            }
          }

          lcd.setCursor(0,1);
          if (elemento == HORA)
          {
            lcd.print("> [OK]:"+m+":"+s);
          }
          if (elemento == MINUTO)
          {
            lcd.print("> "+h+":[OK]:"+s);
          }
          if (elemento == SEGUNDO)
          {
            lcd.print("> "+h+":"+m+":[OK]");
          }
          delay(GLOBALDELAY);
        }
      }
      // Imprimir la pagina seleccionada
      if (navegando)
      {
        switch (pagina)
        {
          case ALARMA_1:
            imprimeAlarma(ALARMA_1,false);
            break;
          case ALARMA_2:
            imprimeAlarma(ALARMA_2,false);
            break;
          case ALARMA_3:
            imprimeAlarma(ALARMA_3,false);
            break;
          case ALARMA_4:
            imprimeAlarma(ALARMA_4,false);
            break;
          case ALARMA_5:
            imprimeAlarma(ALARMA_5,false);
            break;
        }
      }
      delay(GLOBALDELAY);
    }
  }
}

void imprimeAlarma (int a, bool edit)
{

  h = String(alarmas[a][0]);
  m = String(alarmas[a][1]);
  s = String(alarmas[a][2]);

  if(h.length() == 1)
  {
    h = "0" + h;
  }
  if(m.length() == 1)
  {
    m = "0" + m;
  }
  if(s.length() == 1)
  {
    s = "0" + s;
  }

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(pantallasAlarmas[a]);
  lcd.setCursor(0,1);
  if (edit)
  {
    lcd.print("> "+h+":"+m+":"+s);
  } else {
    lcd.print(h+":"+m+":"+s);
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

void calculaComidasSaltadas () {
  for(int i=0; i<5; i++)
  {
    if((alarmas[i][0] != 0 || alarmas[i][1] != 0) && comidasServidas[i] == 0)
    {
      comidasSaltadas++;
    }
  }
}
//
void setup () {
  Serial.begin(9600);
  Wire.begin();
  lcd.init();
  lcd.backlight();

  lcd.createChar(0,pendiente);
  lcd.createChar(1,dispensada);
  lcd.createChar(2,saltada);
  lcd.createChar(3,manual);

  miservo.attach (PIN_SERVO);   // Initialize Servo
  pinMode (PIN_BUZZER, OUTPUT); // Initialize buzzer
  pinMode (PIN_AUX, OUTPUT);
  miservo.write (0);            // Set servo position to 0 degrees.

  rtc.begin();

  DateTime now = rtc.now();
  setTime (now.hour (), now.minute (), now.second (), now.month (), now.day (), now.year ());

  cargarComidasDispensadas();
  
  if(ultimoDiaDispensado != now.day())
  {
    restablecerComidasDispensadas();
  } else {
    calculaComidasSaltadas();
  }

  iniciaComidas();
  imprimeGraficoComidas();
}

void loop () {
  teclado.update();
  HoraFecha = rtc.now();

  segundo = HoraFecha.second();
  minuto  = HoraFecha.minute();
  hora    = HoraFecha.hour();
  dia     = HoraFecha.dayOfTheWeek();
  dd      = HoraFecha.day();
  mes     = HoraFecha.month();
  anio    = HoraFecha.year();

  // Validar el cambio de dia para restablecer el registro de comidas dispensadas.
  if(hora==0 && minuto==0 && segundo==0)
  {
    restablecerComidasDispensadas();
  }

  // Validar la hora actual contra las alarmas para dispensar si es necesario.
  for(int i=0; i<5; i++)
  {
    if(hora == alarmas[i][0] && minuto == alarmas[i][1] && segundo == alarmas[i][2])
    {
      dispensar();
    }
  }

  if (teclado.onPress(ARRIBA))
  {
    if(pagina == paginas-1 && !imprimirGrafico)
    {
      imprimirGrafico = true;
    }
    subir();
  }
  if (teclado.onPress(ABAJO))
  {
    if(pagina == 1 && !imprimirGrafico)
    {
      imprimirGrafico = true;
    }
    bajar();
  }
  if (teclado.onPress(OK))
  {
    if (pagina != 0){
      editando = true;
      while (editando){
        teclado.update();

        muestraPagina();

        if (teclado.onPress(MENU))
        {
          editando = false;
        }
        delay(GLOBALDELAY);
      }
    } else { //

    }
  }
  if (teclado.onPress(MENU))
  {
    if(!imprimirGrafico)
    {
      imprimirGrafico = true;
    }
    if(pagina !=0){
      pagina = 0;
    }
  }

  muestraPagina();

  delay(GLOBALDELAY);
}
