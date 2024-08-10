/*В проекте используется плата Arduino Nano, датчик температуры и влажноси почвы DHT22, энкодер, тактовая кнопка, жидкокристаллический
дисплей 2004 с шиной I2C(Пины A4, A5 - SDA, SCL), датчик влажности почвы FC-28, электромагнитное реле(лучше использовать твердотельное).
Для управления дисплеем используется библиотека LiquidCrystal_I2C, датчиком температуры и влажности возудуха - библиотека DHT, энкодером и кнопкой = 
библиотеки GyverEncoder и GyverButton*/

#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <GyverEncoder.h>
#include <GyverButton.h>

// Encoder pins(CLK, DT, SW)
const uint8_t SW_PIN = 3;
const uint8_t CLK_PIN = 5;
const uint8_t DT_PIN = 4;

// DHT22 pin(IN/S)
const uint8_t DHT_PIN = 6;

// Relay pin (S)
const uint8_t RELAY1_PIN = 7;

// Button pin
const uint8_t BTN_PIN = 8;

// Параметры, в зависимости от которых включается полив в авторежиме.
// Минимальная влажность почвы, ниже которой включаем полив
int8_t minimum_soil_humidity = 0;

// Минимальные температура и влажность воздуха, выше и ниже которых соответственно включаем полив
int8_t minimum_temperature = 30;
int8_t minimum_humidity = 30;

// Время полива в минутах(максимум 60)
int16_t watering_time = 1;

// Температура и влажность воздуха, влажность почвы
int8_t temperature;
int8_t humidity;
int8_t soil_humidity;

// Флаг неиспользуемый, но необходимый в случае доработки системы для реализации сна и энергосбережения(при необходимости)
bool system_activated = false;

// Флаг включения/выключения авторежима
bool auto_mode = true;

// Флаг открытия/закрытия меню настроек
bool settings_opend = false;

// Параметр, настраиваемый в данный момент
int8_t settings_mode = 0;


// Датчик температуры и влажности
DHT dht(DHT_PIN, DHT22);

// Энкодер
Encoder enc(CLK_PIN, DT_PIN, SW_PIN);

// Кнопка
GButton btn(BTN_PIN);

// Флаг состояния реле
bool relay1_on = false;

// Жидкокристаллический дисплей 2004, подключенный по шине I2C
LiquidCrystal_I2C lcd(0x27, 20, 4);

// Таймер отсчета времени для обновления данных
uint32_t timer;

// Таймер для отключения реле в авторежиме
uint32_t timer_2;


// Функция считывания данных
void read_data(){
  // Температура и влажность считываются датчиком и преобразуются в целое число
  temperature = (int8_t)dht.readTemperature();
  humidity = (int8_t)dht.readHumidity();

  // Влажность почвы считывается датчиком(FC-28) с аналогового входа A2 и переводится в проценты
  soil_humidity = map(analogRead(A2), 1024, 0, 0, 100);
}

/*Система активируется в случае, если влажность почвы ниже минимальной или температура воздуха превышает минимальную, а влажность
воздуха ниже минимальной. При необходимости можно ввести флаг состояния второго реле, добавить еще один таймер и еще одну переменную,
для времени опрыскивания и, добавив второе реле управлять второй помпой, отвечающей за опрыскивание*/
void activate_system(){
  if(soil_humidity < minimum_soil_humidity || temperature > minimum_temperature && humidity < minimum_humidity){
    relay1_on = true;
  }
}

// Функция вывода информации на дислей
void print_data(){
  lcd.clear();
  if(!settings_opend){
    lcd.setCursor(0, 0);
    lcd.print("Temperature: ");
    lcd.print(temperature);
    lcd.print("(C)");
    lcd.setCursor(0, 1);
    lcd.print("Humidity: ");
    lcd.print(humidity);
    lcd.print("%");

    lcd.setCursor(0, 2);
    lcd.print("Soil humidity: ");
    lcd.print(soil_humidity);
    lcd.print("%");

    if(auto_mode){
      lcd.setCursor(0, 3);
      lcd.print("Mode: AUTO");
    }
    else{
      lcd.setCursor(0, 3);
      lcd.print("Mode: HANDMODE");
    }
  }
  else{
    switch(settings_mode){
      case 0:
        lcd.setCursor(0, 0);
        lcd.print(">Min temp: ");
        lcd.print(minimum_temperature);
        lcd.print("(C)<");

        lcd.setCursor(0, 1);
        lcd.print("Min hum: ");
        lcd.print(minimum_humidity);
        lcd.print("%");

        lcd.setCursor(0, 2);
        lcd.print("Min soil hum: ");
        lcd.print(minimum_soil_humidity);
        lcd.print("%");

        lcd.setCursor(0, 3);
        lcd.print("Wat ime: ");
        lcd.print(watering_time);
        lcd.print(" min");
        break;
      case 1:
        lcd.setCursor(0, 0);
        lcd.print("Min temp: ");
        lcd.print(minimum_temperature);
        lcd.print("(C)");

        lcd.setCursor(0, 1);
        lcd.print(">Min hum: ");
        lcd.print(minimum_humidity);
        lcd.print("%<");

        lcd.setCursor(0, 2);
        lcd.print("Min soil hum: ");
        lcd.print(minimum_soil_humidity);
        lcd.print("%");

        lcd.setCursor(0, 3);
        lcd.print("Wat ime: ");
        lcd.print(watering_time);
        lcd.print(" min");
        break;
      case 2:
        lcd.setCursor(0, 0);
        lcd.print("Min temp: ");
        lcd.print(minimum_temperature);
        lcd.print("(C)");

        lcd.setCursor(0, 1);
        lcd.print("Min hum: ");
        lcd.print(minimum_humidity);
        lcd.print("%");

        lcd.setCursor(0, 2);
        lcd.print(">Min soil hum: ");
        lcd.print(minimum_soil_humidity);
        lcd.print("%<");

        lcd.setCursor(0, 3);
        lcd.print("Wat ime: ");
        lcd.print(watering_time);
        lcd.print(" min");
        break;
      case 3:
        lcd.setCursor(0, 0);
        lcd.print("Min temp: ");
        lcd.print(minimum_temperature);
        lcd.print("(C)");

        lcd.setCursor(0, 1);
        lcd.print("Min hum: ");
        lcd.print(minimum_humidity);
        lcd.print("%");

        lcd.setCursor(0, 2);
        lcd.print("Min soil hum: ");
        lcd.print(minimum_soil_humidity);
        lcd.print("%");

        lcd.setCursor(0, 3);
        lcd.print(">Wat ime: ");
        lcd.print(watering_time);
        lcd.print(" min<");
        break;
      case 4:
        lcd.setCursor(0, 0);
        lcd.print(">Mode: ");
        if(auto_mode){
          lcd.print("AUTO<");
        }
        else{
          lcd.print("HANDMODE<");
        }

        lcd.setCursor(0, 1);
        lcd.print("Save and exit");
        break;
      case 5:
        lcd.setCursor(0, 0);
        lcd.print("Mode: ");
        if(auto_mode){
          lcd.print("AUTO");
        }
        else{
          lcd.print("HANDMODE");
        }

        lcd.setCursor(0, 1);
        lcd.print(">Save and exit<");
        break;
    }
  }
}

void setup(){
  // Инициализация дисплея и включение подсветки
  lcd.init();
  lcd.backlight();

  // Инициализация энкодера и датчика температуры и влажности
  enc.setType(TYPE2);
  dht.begin();

  // Считывание данных, вывод их на экран и активация полива в случае необъодимости при первом включении
  read_data();
  print_data();
  pinMode(RELAY1_PIN, OUTPUT);
  activate_system();
}

void loop(){
  // Необходимо для корректной работы энкодера и кнопки
  enc.tick();
  btn.tick();

  // В ручном режиме нажатие кнопки включает полив
  if(btn.isPress()){
    if(!auto_mode){
      relay1_on = !relay1_on;
    }
  }

  // Данные обновляются раз в 3 часа
  if(millis() - timer > (uint32_t)3 * 60 * 60 * 1000){
    timer = millis();
    read_data();
    print_data();
    if(auto_mode && !settings_opend){
      activate_system();
    }
  }

  /*По нажатию кнопки энкодера открывается меню настроек. Далее нажатие кнопки энкодера переключает настраиваемый параметр до тех пор, пока 
  не будет выполнено Сохранить и выйти*/

  if(enc.isClick()){
    if(!settings_opend){
      settings_opend = true;
      settings_mode = 0;
      print_data();
    }
    else{
      settings_mode++;
      print_data();
      if(settings_mode == 6){
        settings_opend = false;
        read_data();
        print_data();
      }
    }
  }
  
  // Вращение энкодера позволяет прибавлять или убавлять настраиваемый параметр
  if(enc.isLeft()){
    if(settings_opend){
      switch(settings_mode){
        case 0:
          minimum_temperature++;
          if(minimum_temperature > 50){
            minimum_temperature = 0;
          }
          print_data();
          break;
        case 1:
          minimum_humidity++;
          if(minimum_humidity > 100){
            minimum_humidity = 0;
          }
          print_data();
          break;
        case 2:
          minimum_soil_humidity++;
          if(minimum_soil_humidity > 100){
            minimum_soil_humidity = 0;
          }
          print_data();
          break;
        case 3:
          watering_time++;
          if(watering_time > 60){
            watering_time = 0;
          }
          print_data();
          break;
        case 4:
          auto_mode = !auto_mode;
          print_data();
          break;
      }
    }
  }

  if(enc.isRight()){
    if(settings_opend){
      switch(settings_mode){
        case 0:
          minimum_temperature--;
          if(minimum_temperature < 0){
            minimum_temperature = 50;
          }
          print_data();
          break;
        case 1:
          minimum_humidity--;
          if(minimum_humidity < 0){
            minimum_humidity = 100;
          }
          print_data();
          break;
        case 2:
          minimum_soil_humidity--;
          if(minimum_soil_humidity < 0){
            minimum_soil_humidity = 100;
          }
          print_data();
          break;
        case 3:
          watering_time--;
          if(watering_time < 0){
            watering_time = 60;
          }
          print_data();
          break;
        case 4:
          auto_mode = !auto_mode;
          print_data();
          break;
      }
    }
  }

  // Таймер, который следит за отключением системы по истечение времени полива
  if(millis() - timer_2 > (uint32_t)watering_time * 60 * 1000){
    timer_2 = millis();
    if(relay1_on && auto_mode){
      relay1_on = !relay1_on;
    }
  }

  // Включение/выключение реле
  if(relay1_on){
    digitalWrite(RELAY1_PIN, HIGH);
  }
  else{
    digitalWrite(RELAY1_PIN, LOW);
  }
}